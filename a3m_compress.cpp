/*
 * Adapted from HH-suite 3.0: a3m_compress.h
 * Original Author: meiermark
 */


#include "a3m_compress.h"
#include <sstream>

void readU16(const char **ptr, uint16_t &result) {
    unsigned char array[2];

    array[0] = (unsigned char) (**ptr);
    (*ptr)++;
    array[1] = (unsigned char) (**ptr);
    (*ptr)++;

    result = array[0] | (array[1] << 8);
}


void readU32(const char **ptr, uint32_t &result) {
    unsigned char array[4];

    array[0] = (unsigned char) (**ptr);
    (*ptr)++;
    array[1] = (unsigned char) (**ptr);
    (*ptr)++;
    array[2] = (unsigned char) (**ptr);
    (*ptr)++;
    array[3] = (unsigned char) (**ptr);
    (*ptr)++;

    result = array[0] | (array[1] << 8) | (array[2] << 16) | (array[3] << 24);
}

void extract_a3m(const char *data, size_t data_size,
                 Php::Object *sequenceReader,
                 Php::Object *headerReader,
                 std::ostream &output) {

    //read stuff till compressed part
    char last_char = '\0';
    size_t index = 0;
    size_t consensus_length = 0;
    char inConsensus = 0;

    //handle commentary line
    if ((*data) == '#') {
        while ((*data) != '\n') {
            output.put((*data));
            last_char = (*data);
            data++;
            index++;
        }

        output.put('\n');
        last_char = '\n';
        data++;
        index++;
    }

    while (!(last_char == '\n' && (*data) == ';') && index < data_size) {
        if ((*data) == '\n') {
            inConsensus++;
        }
        else if (inConsensus == 1) {
            consensus_length++;
        }

        output.put((*data));
        last_char = (*data);
        data++;
        index++;
    }

    //get past ';'
    data++;
    index++;

    while (index < data_size - 1) {
        unsigned int entry_index;
        unsigned short int nr_blocks;
        unsigned short int start_pos;

        readU32(&data, entry_index);
        index += 4;

        std::string sequence = sequenceReader->call("getData", static_cast<int64_t>(entry_index));
        std::string header = headerReader->call("getData", static_cast<int64_t>(entry_index));

        // make sure we always have a valid fasta prefix
        if (header[0] != '>') {
            output.put('>');
        }

        output.write(header.c_str(), header.length() - 1);
        output.put('\n');

        readU16(&data, start_pos);
        index += 2;

        readU16(&data, nr_blocks);
        index += 2;

        size_t actual_pos = start_pos;
        size_t alignment_length = 0;
        for (unsigned short int block_index = 0; block_index < nr_blocks; block_index++) {
            unsigned char nr_matches = (unsigned char) (*data);
            data++;
            index++;

            for (int i = 0; i < nr_matches; i++) {
                output.put(sequence[actual_pos - 1]);
                actual_pos++;
                alignment_length++;
            }

            char nr_insertions_deletions = (*data);
            data++;
            index++;

            if (nr_insertions_deletions > 0) {
                for (int i = 0; i < nr_insertions_deletions; i++) {
                    output.put(tolower(sequence[actual_pos - 1]));
                    actual_pos++;
                }
            }
            else {
                for (int i = 0; i < -nr_insertions_deletions; i++) {
                    output.put('-');
                    alignment_length++;
                }
            }
        }

        while (alignment_length < consensus_length) {
            output.put('-');
            alignment_length++;
        }

        output.put('\n');
    }
}


void A3mExtractor::__construct(Php::Parameters &params) {
    if (params.size() < 4) {
        throw Php::Exception("Not enough parameters");
    }

    headerReader = new Php::Object("IntDBReader", (const char *) params[0], (const char *) params[1], 1);
    sequenceReader = new Php::Object("IntDBReader", (const char *) params[2], (const char *) params[3], 1);
}

void A3mExtractor::__destruct() {
    delete headerReader;
    delete sequenceReader;
}

Php::Value A3mExtractor::readCompressedA3M(Php::Parameters &params) {
    if (params.size() < 1) {
        throw Php::Exception("Not enough parameters");
    }

    std::ostringstream ss;
    std::string ca3m = (const char *) params[0];
    extract_a3m(ca3m.c_str(), ca3m.length(), sequenceReader, headerReader, ss);
    std::string a3m = ss.str();

    return a3m;
}

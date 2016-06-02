/*
 * Adapted from HH-suite 3.0: a3m_compress.h
 * Original Author: meiermark
 */


#include "a3m_compress.h"
#include <sys/mman.h>
#include <sstream>

void writeU16(std::ostream &file, uint16_t val) {
    unsigned char bytes[2];

    // extract the individual bytes from our value
    bytes[0] = (val) & 0xFF;  // low byte
    bytes[1] = (val >> 8) & 0xFF;  // high byte

    // write those bytes to the file
    file.write((char *) bytes, 2);
}

void readU16(const char **ptr, uint16_t &result) {
    unsigned char array[2];

    array[0] = (unsigned char) (**ptr);
    (*ptr)++;
    array[1] = (unsigned char) (**ptr);
    (*ptr)++;

    result = array[0] | (array[1] << 8);
}

void writeU32(std::ostream &file, uint32_t val) {
    unsigned char bytes[4];

    // extract the individual bytes from our value
    bytes[0] = (val) & 0xFF;
    bytes[1] = (val >> 8) & 0xFF;
    bytes[2] = (val >> 16) & 0xFF;
    bytes[3] = (val >> 24) & 0xFF;

    // write those bytes to the file
    file.write((char *) bytes, 4);
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
                 ffindex_index_t *ffindex_sequence_database_index, char *ffindex_sequence_database_data,
                 ffindex_index_t *ffindex_header_database_index, char *ffindex_header_database_data,
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

        ffindex_entry_t *sequence_entry = ffindex_get_entry_by_index(ffindex_sequence_database_index, entry_index);
        char *sequence = ffindex_get_data_by_entry(ffindex_sequence_database_data, sequence_entry);

        ffindex_entry_t *header_entry = ffindex_get_entry_by_index(ffindex_header_database_index, entry_index);
        char *header = ffindex_get_data_by_entry(ffindex_header_database_data, header_entry);

        // make sure we always have a valid fasta prefix
        if (header[0] != '>') {
            output.put('>');
        }

        output.write(header, header_entry->length - 1);
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

    std::string sequenceDataFile = (const char *) params[0];
    std::string sequenceIndexFile = (const char *) params[1];
    std::string headerDataFile = (const char *) params[2];
    std::string headerIndexFile = (const char *) params[3];

    sequenceDataHandle = fopen(sequenceDataFile.c_str(), "r");
    if (sequenceDataHandle == NULL) {
        std::ostringstream message;
        message << "ERROR: Could not open ffindex sequence data file! (" << sequenceDataFile.c_str() << ")!";
        throw Php::Exception(message.str());
    }

    sequenceIndexHandle = fopen(sequenceIndexFile.c_str(), "r");
    if (sequenceIndexHandle == NULL) {
        std::ostringstream message;
        message << "ERROR: Could not open ffindex sequence index file! (" << sequenceIndexFile.c_str() << ")!";
        throw Php::Exception(message.str());
    }

    sequenceData = ffindex_mmap_data(sequenceDataHandle, &sequenceSize);
    size_t entries = ffcount_lines(sequenceIndexFile.c_str());
    sequenceIndex = ffindex_index_parse(sequenceIndexHandle, entries);
    if (sequenceIndex == NULL) {
        throw Php::Exception("ERROR: Sequence index could not be loaded!");
    }

    //prepare ffindex header database
    headerDataHandle = fopen(headerDataFile.c_str(), "r");
    if (headerDataHandle == NULL) {
        std::ostringstream message;
        message << "ERROR: Could not open ffindex header data file! (" << headerDataFile.c_str() << ")!";
        throw Php::Exception(message.str());
    }

    headerIndexHandle = fopen(headerIndexFile.c_str(), "r");
    if (headerIndexHandle == NULL) {
        std::ostringstream message;
        message << "ERROR: Could not open ffindex header index file! (" << headerIndexFile.c_str() << ")!";
        throw Php::Exception(message.str());
    }

    headerData = ffindex_mmap_data(headerDataHandle, &headerSize);
    entries = ffcount_lines(headerIndexFile.c_str());
    headerIndex = ffindex_index_parse(headerIndexHandle, entries);

    if (headerIndex == NULL) {
        std::ostringstream message;
        message << "Header index could not be loaded!";
        throw Php::Exception(message.str());
    }
}

void A3mExtractor::__destruct() {
    munmap(sequenceIndex->index_data, sequenceIndex->index_data_size);
    free(sequenceIndex);

    munmap(sequenceData, sequenceSize);

    if (sequenceIndexHandle) {
        fclose(sequenceIndexHandle);
    }

    if (sequenceDataHandle) {
        fclose(sequenceDataHandle);
    }

    munmap(headerIndex->index_data, headerIndex->index_data_size);
    free(headerIndex);

    munmap(headerData, headerSize);

    if (headerIndexHandle) {
        fclose(headerIndexHandle);
    }

    if (headerDataHandle) {
        fclose(headerDataHandle);
    }
}

Php::Value A3mExtractor::readCompressedA3M(Php::Parameters &params) {
    if (params.size() < 1) {
        throw Php::Exception("Not enough parameters");
    }

    std::ostringstream ss;
    std::string ca3m = (const char *) params[0];
    extract_a3m(ca3m.c_str(), ca3m.length(), sequenceIndex, sequenceData, headerIndex, headerData, ss);
    std::string a3m = ss.str();

    return a3m;
}

#include "a3m_reader.h"

#include "kseq/kseq.h"
#include "kseq/kseq_buffer_reader.h"

#include <cstddef>
#include <vector>
#include <sstream>
#include <algorithm>

#include <unistd.h>
#include <cstdio>
#include <iterator>

KSEQ_INIT(kseq_buffer_t*, kseq_buffer_reader)

enum HMMState {
    Deletion = 0,
    Insertion,
    Match
};

HMMState getState(char r) {
    if (r >= 'a' && r <= 'z') {
        return Insertion;
    } else if (r >= 'A' && r <= 'Z') {
        return Match;
    }

    switch (r) {
        case '-':
            return Deletion;
        case '.':
            return Insertion;
        default:
            return Match;
    }
}

void addInsert(std::vector<char> &sequence, size_t col) {
    sequence.insert(sequence.begin() + col, '.');
}

char translateA2M(char b) {
    if ((b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z')) {
        return b;
    }

    switch (b) {
        case '*':
            return 0;
        case '-':
            return '-';
        case '.':
            return '.';
        case '/':
            return '-';
        default:
            return 0;
    }
}

bool columnHasInsertion(const std::vector<std::vector<char>> &entries, size_t col) {
    for (std::vector<std::vector<char>>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        if (getState((*it)[col]) == Insertion) {
            return true;
        }
    }
    return false;
}

void addSequence(std::vector<std::vector<char>> &entries, const std::string &sequence, size_t &length) {
    if (sequence.size() == 0) {
        return;
    }

    // We will, in all likelihood, modify the sequence if it's in A3M format.
    // So we copy it to prevent weird effects to the caller.
    std::vector<char> copy(sequence.begin(), sequence.end());
    for (std::vector<char>::iterator it = copy.begin(); it != copy.end(); ++it) {
        (*it) = translateA2M(*it);
    }

    // The first sequence is easy.
    if (entries.size() == 0) {
        entries.push_back(copy);
        length = sequence.size();
        return;
    }

    // This should be an A3M formatted sequence (no way to do a sanity check
    // though, I don't think).
    // Therefore, we need to assimilate it with other sequences, which will
    // change the length of the MSA. In particular, we'll need to add '.'
    // residues to existing sequences.
    for (size_t col = 0; col < length; ++col) {
        bool colHasInsert = columnHasInsertion(entries, col);
        if (col >= copy.size()) {
            if (colHasInsert) {
                copy.push_back('.');
            } else {
                copy.push_back('-');
            }
            continue;
        }

        bool seqHasInsert = getState(copy[col]) == Insertion;
        if (colHasInsert && seqHasInsert) {
            // do nothing, we're in sync
        } else if (colHasInsert) {
            // Put an insert into the sequence we're adding.
            addInsert(copy, col);
        } else if (seqHasInsert) {
            // Put an insert into the rest of the sequences.
            for (std::vector<std::vector<char>>::iterator it = entries.begin(); it != entries.end(); ++it) {
                addInsert(*it, col);
            }

            length++;
        }
    }
    entries.push_back(copy);
}


std::string a3mtofasta(std::string a3m) {
    std::vector<std::string> headers;
    std::vector<std::vector<char>> entries;
    size_t length;

    kseq_buffer_t buffer(const_cast<char *>(a3m.c_str()), a3m.length());
    kseq_t *seq = kseq_init(&buffer);
    while (kseq_read(seq) >= 0) {
        std::string header = seq->name.s;
        header.append(" ");
        header.append(seq->comment.s);
        headers.push_back(header);

        std::string sequence = seq->seq.s;
        addSequence(entries, sequence, length);
    }
    kseq_destroy(seq);

    if (entries.size() < 1) {
        return std::string();
    }

    const std::vector<char> &query = entries[0];

    std::ostringstream ss;
    for (size_t i = 0; i < entries.size(); ++i) {
        ss << ">" << headers[i] << "\n";

        const std::vector<char> &entry = entries[i];
        for (size_t i = 0; i < query.size(); ++i) {
            if (query[i] == '.' || query[i] == '-') {
                continue;
            } else {
                if (entry[i] == '.') {
                    ss << '-';
                } else {
                    ss << static_cast<char>(toupper(entry[i]));
                }
            }
        }

        ss << "\n";
    }

    return ss.str();
}

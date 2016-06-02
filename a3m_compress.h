#ifndef A3M_EXTRACTOR_H
#define A3M_EXTRACTOR_H

#include <phpcpp.h>

extern "C" {
#include <ffindex.h>
#include <ffutil.h>
}

class A3mExtractor : public Php::Base {
public:
    void __construct(Php::Parameters &params);

    void __destruct();

    Php::Value readCompressedA3M(Php::Parameters &params);

private:
    ffindex_index_t *sequenceIndex, *headerIndex;
    char *sequenceData, *headerData;
    size_t sequenceSize, headerSize;
    FILE *sequenceDataHandle, *sequenceIndexHandle, *headerDataHandle, *headerIndexHandle;
};

#endif

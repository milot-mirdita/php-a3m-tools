#ifndef A3M_EXTRACTOR_H
#define A3M_EXTRACTOR_H

#include <phpcpp.h>

class A3mExtractor : public Php::Base {
public:
    void __construct(Php::Parameters &params);

    void __destruct();

    Php::Value readCompressedA3M(Php::Parameters &params);

private:
    Php::Object *headerReader, *sequenceReader;
};

#endif

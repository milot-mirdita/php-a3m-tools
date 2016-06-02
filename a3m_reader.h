#ifndef PHP_A3M_READER_H
#define PHP_A3M_READER_H

#include <phpcpp.h>

#include <cstddef>
#include <vector>
#include <string>

class A3mReader : public Php::Base {
public:
    void __construct(Php::Parameters &params);

    Php::Value getFasta();

private:
    void addSequence(const std::string& sequence);

    bool columnHasInsertion(size_t col);

    std::vector<std::string> headers;
    std::vector<std::vector<char>> entries;
    size_t length;
};

#endif

#include "a3m_compress.h"

extern "C" {
    
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  structure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        static Php::Extension extension("A3mExtractor", "0.1");

        Php::Class<A3mExtractor> extractor("A3mExtractor");
        extractor.method("__construct", &A3mExtractor::__construct);
        extractor.method("__destruct", &A3mExtractor::__destruct);
        extractor.method("readCompressedA3M", &A3mExtractor::readCompressedA3M);

        extension.add(std::move(extractor));

        return extension;
    }
}

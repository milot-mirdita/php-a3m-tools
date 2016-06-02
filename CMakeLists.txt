cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(php_a3m_extractor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(php_a3m_extractor_source_files
        a3m_compress.h
        a3m_compress.cpp
        main.cpp)

add_library(a3mextractor SHARED ${php_a3m_extractor_source_files})
target_link_libraries(a3mextractor phpcpp)
set_target_properties(a3mextractor
        PROPERTIES
        PREFIX ""
        SUFFIX ".so")

execute_process(COMMAND php-config --extension-dir
        OUTPUT_VARIABLE PHP_EXTENSION_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)

install(TARGETS a3mextractor LIBRARY DESTINATION ${PHP_EXTENSION_DIR})

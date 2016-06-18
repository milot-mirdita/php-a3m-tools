cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(php_a3m_tools)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(php_a3m_tools_source_files
        a3m_extractor.h
        a3m_extractor.cpp
        a3m_reader.cpp
        a3m_reader.h
        main.cpp
        )

add_library(a3mtools SHARED ${php_a3m_tools_source_files})
target_link_libraries(a3mtools phpcpp)
set_target_properties(a3mtools
        PROPERTIES
        PREFIX ""
        SUFFIX ".so")

execute_process(COMMAND php-config --extension-dir
        OUTPUT_VARIABLE PHP_EXTENSION_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)

install(TARGETS a3mtools LIBRARY DESTINATION ${PHP_EXTENSION_DIR})

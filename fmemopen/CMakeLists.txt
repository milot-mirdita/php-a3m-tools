include(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
check_function_exists(fmemopen HAVE_FMEMOPEN)

if (HAVE_FMEMOPEN)
    add_definitions(-DHAVE_FMEMOPEN=1)
else()
    set(FMEMOPEN_SRC
            fmemopen.c
            fmemopen.h
            )
    add_library(fmemopen_polyfill ${FMEMOPEN_SRC})
endif ()

project(common)

set(src
        base64.h
        base64.cpp
        sha1.h
        sha1.cpp
        )

add_library(common STATIC ${src})
project(http)

set(src
        main.cpp
        )

add_executable(http ${src})

target_include_directories(http
        PRIVATE ../xtcp)

target_link_libraries(http
        PRIVATE pthread
        PRIVATE m
        PRIVATE xtcp)

link_directories(../xtcp/)
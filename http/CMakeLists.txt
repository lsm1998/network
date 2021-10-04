project(http)

set(src
        main.cpp
        epoll.cpp
        select.cpp
        http_common.h
        http_common.cpp
        )

add_executable(http ${src})

target_include_directories(http
        PRIVATE ../xtcp)

target_link_libraries(http
        PRIVATE xtcp)

if(UNIX OR Linux)
    target_link_libraries(http PRIVATE pthread)
endif()

link_directories(../xtcp/)
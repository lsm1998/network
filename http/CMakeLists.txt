project(http)

set(src
        main.cpp
        epoll.cpp
        select.cpp
        common.h
        common.cpp
        http_request.h
        http_request.cpp
        http_response.h
        http_response.cpp
        websocket.cpp
        websocket.h
        frame.cpp
        frame.h
        )

add_executable(http ${src})

target_include_directories(http
        PRIVATE ../xtcp
        PRIVATE ../common)

target_link_libraries(http
        PRIVATE xtcp
        PRIVATE common)

if (UNIX OR Linux)
    target_link_libraries(http PRIVATE pthread)
endif ()

link_directories(../xtcp/)
link_directories(../common/)
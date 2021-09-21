project(websocket)

set(src
        main.cpp
        websocket_request.cpp
        websocket_request.h
        websocket_respond.cpp
        websocket_respond.h
        debug_log.cpp
        debug_log.h
        websocket_handler.cpp
        websocket_handler.h
        network_interface.cpp
        network_interface.h
        )

add_executable(websocket ${src})

if(UNIX OR Linux)
    target_link_libraries(websocket PRIVATE pthread)
endif()

target_include_directories(websocket
        PRIVATE ../common)

target_link_libraries(websocket
        PRIVATE common)

link_directories(../common/)
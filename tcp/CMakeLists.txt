project(tcp)

set(src
        socket.cpp
        socket.h
        server_socket.cpp
        server_socket.h
        )

add_executable(tcp server.cpp)

add_executable(client client.cpp)

if(UNIX OR Linux)
    target_link_libraries(tcp pthread)
    target_link_libraries(client pthread)
endif()
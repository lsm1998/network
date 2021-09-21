cmake_minimum_required(VERSION 3.14)
project(network)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(websocket)
add_subdirectory(http)
add_subdirectory(tcp)
add_subdirectory(xtcp)
add_subdirectory(base)
add_subdirectory(common)

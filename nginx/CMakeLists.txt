project(nginx)

include_directories(include)

add_subdirectory(src)

message('nginx...')

if (UNIX OR Linux)
    target_link_libraries(nginx PRIVATE pthread)
endif ()

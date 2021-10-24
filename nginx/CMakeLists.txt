project(nginx)


include_directories(include)

add_subdirectory(src)



if (UNIX OR Linux)
    target_link_libraries(http PRIVATE pthread)
endif ()

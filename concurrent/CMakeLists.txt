project(concurrent)

add_executable(concurrent_hello hello.cpp)

if (UNIX OR Linux)
    target_link_libraries(concurrent_hello PRIVATE pthread)
endif ()
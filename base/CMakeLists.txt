project(base)

set(src
        main.cpp
        main.h
        initializer.h
        struct_bind.cpp
        smart_point.cpp
        thread.cpp
        my_smart_point.cpp
        my_smart_point.h
        obj_size.cpp
        ../ae/main.cpp)

add_executable(base ${src})

if(UNIX OR Linux)
    target_link_libraries(base pthread)
endif()
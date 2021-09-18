project(base)

set(src
        main.cpp
        main.h
        initializer.h
        struct_bind.cpp
        smart_point.cpp
        thread.cpp
        )

add_executable(base ${src})
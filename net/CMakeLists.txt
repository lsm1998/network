project(net)

set(src
        main.cpp
        net.h
        ae.h
        ae.cpp
        epoll.cpp
        )

add_executable(net ${src})
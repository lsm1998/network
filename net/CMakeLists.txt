project(net)

set(src
        main.cpp
        net.h
        ae.h
        ae.cpp
        ae_epoll.cpp
        ae_select.cpp
        epoll_demo.cpp
        select_demo.cpp
        )

add_executable(net ${src})
project(net)

set(src
        main.cpp
        sock.h
        sock.cpp
        ae.h
        ae.cpp
        ae_epoll.cpp
        ae_select.cpp
        ae_kqueue.cpp
        epoll_demo.cpp
        select_demo.cpp
        anet.cpp
        anet.h)

add_executable(net ${src})
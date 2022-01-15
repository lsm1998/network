project(ae)

set(src main.cpp ae.h ae.cpp ae_kqueue.cpp ae_kqueue.h anet.h anet.cpp)

add_executable(ae ${src})
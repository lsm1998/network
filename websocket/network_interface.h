#ifndef __NETWORK_INTERFACE__
#define __NETWORK_INTERFACE__

#include "websocket_handler.h"
#ifdef __linux__
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#elif WIN32
#include <windows.h>
#define close closesocket
#endif
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include "debug_log.h"

#define PORT 9000
#define TIME_WAIT 100
#define BUFF_LEN 2048
#define MAX_EVENTS_SIZE 20

typedef std::map<int, Websocket_Handler *> WEB_SOCKET_HANDLER_MAP;

class Network_Interface
{
private:
    Network_Interface();

    ~Network_Interface();

    int init();

    int epoll_loop();

    int set_noblock(int fd);

    void ctl_event(int fd, bool flag);

public:
    void run();

    static Network_Interface *get_share_network_interface();

private:
    int epollfd_;
    int listenfd_;
    WEB_SOCKET_HANDLER_MAP websocket_handler_map_;
    static Network_Interface *m_network_interface;
};

#define NETWORK_INTERFACE Network_Interface::get_share_network_interface()

#endif

//
// Created by Administrator on 2021/12/7.
//

#include "main.h"

struct Server
{
    int port;
    int ipfd[16]; /* TCP socket file descriptors */
    int count;
    char **bindaddr;
    int bindaddr_count;
    int tcp_backlog;
};

int main()
{
    Server server{};
    server.port = 8888;
    server.count = 0;
    server.bindaddr_count = 0;

    int result = listenToPort(server.port, server.ipfd, &server.count, server.bindaddr, server.bindaddr_count,
                              server.tcp_backlog);
    if (result < 0)
    {
        perror("listenToPort fail");
        return -1;
    }
    aeEventLoop *eventLoop = aeCreateEventLoop(1024);
    aeMain(eventLoop);
}
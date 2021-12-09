//
// Created by Administrator on 2021/12/8.
//
#include "net.h"

#ifdef HAVE_SELECT

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>

constexpr int FD_SET_SIZE = 10;

void select_demo()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sockAddrIn = {};
    sockAddrIn.sin_port = htons(8888);
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_addr.s_addr = htonl(0);
    socklen_t len = sizeof(sockAddrIn);
    if (bind(sock_fd, reinterpret_cast<const sockaddr *>(&sockAddrIn), len) < 0)
    {
        perror("bind fail");
        return;
    }
    if (listen(sock_fd, 511) < 0)
    {
        perror("listen fail");
        return;
    }

    // select fd 集合
    fd_set read_fds;
    FD_ZERO(&read_fds);

    timeval _timeval = {};
    _timeval.tv_sec = 1;
    _timeval.tv_usec = 0;

    while (true)
    {
        FD_SET(sock_fd, &read_fds);
        int count = select(FD_SET_SIZE, &read_fds, nullptr, nullptr, &_timeval);
        if (count < 0)
        {
            perror("select fail");
            break;
        }
        if (!FD_ISSET(sock_fd, &read_fds))
        {
            continue;
        }

        struct sockaddr_in client_adder{};
        // 如果服务器fd可用，则为accept
        socklen_t client_adder_len = sizeof(struct sockaddr_in);
        int client_fd = accept(sock_fd, (struct sockaddr *) &client_adder, &client_adder_len);
        if (client_fd == -1)
        {
            perror("accept fail");
            return;
        } else
        {
            FD_SET(client_fd, &read_fds);
            printf("%s connect at fd %d\n", inet_ntoa(client_adder.sin_addr), client_fd);
        }
        char buf[1024];
        // 依次查询
        for (int fd = 0; fd < FD_SET_SIZE; fd++)
        {
            if (FD_ISSET(fd, &read_fds))
            {
                // 如果是client活动，进行echo
                int len = read(fd, buf, 1024);
                if (len > 0)
                {
                    write(fd, buf, len);
                } else if (len == -1)
                {
                    // 结束
                    printf("client end at %d\n", client_fd);
                    FD_CLR(fd, &read_fds);
                }
            }
        }
    }
}

#endif
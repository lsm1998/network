//
// Created by Administrator on 2021/12/8.
//
#include "net.h"
#ifdef HAVE_EPOLL
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

/**
 * epoll 标准流程
 */
void epoll_demo()
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

    struct epoll_event event{}, events[1024];
    int epoll_fd = epoll_create(1024);

    event.data.fd = sock_fd;
    // 边缘触发
    event.events = EPOLLET | EPOLLIN;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, events);

    printf("start! \n");

    while (true)
    {
        int count = epoll_wait(epoll_fd, events, 1024, 500);
        if (count < 0)
        {
            break;
        }
        for (int i = 0; i < count; ++i)
        {
            if (events[i].data.fd == sock_fd) // 新的连接
            {
                sockaddr_in clientAddr = {};
                socklen_t len = sizeof(clientAddr);
                int conn_fd = accept(sock_fd, reinterpret_cast<sockaddr *>(&clientAddr), &len);
                event = {};
                event.data.fd = conn_fd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event); //将新的fd添加到epoll的监听队列中
                printf("客户端加入\n");
            } else if (events[i].events & EPOLLIN) // 接收到数据，读socket
            {
                char buf[1024];
                int n = read(sock_fd, buf, 1024);  //读
                buf[n] = '\0';
                // 自定义数据
                event = {};
                event.data.ptr = buf;
                // 设置为可写
                event.events = EPOLLOUT | EPOLLET;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event);//修改标识符，等待下一个循环时发送数据，异步处理的精髓
            } else if (events[i].events & EPOLLOUT) // 有数据待发送，写socket
            {
                // 取数据
                char *data = (char *) events[i].data.ptr;
                write(events[i].data.fd, data, strlen(data));
                event = {};
                // 设置为可读
                event.events = EPOLLIN;
                event.data.fd = events[i].data.fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &event); //修改标识符，等待下一个循环时接收数据
            } else if (events[i].events & EPOLLHUP)
            {
                event = {};
                event.events = EPOLLHUP;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                printf("客户端关闭\n");
            } else
            {
                printf("未实现的处理事件:%d\n", events[i].events);
            }
        }
    }
}
#endif
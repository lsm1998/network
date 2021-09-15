//
// Created by Administrator on 2021/9/13.
//

#include <sys/epoll.h>
#include <xtcp.h>

void epollServer()
{
    auto *server = new XTcp(8888);
    server->CreateSocket();
    server->Bind();

    int epFd = epoll_create(256);
    epoll_event ev = {};
    ev.data.fd = server->sockFd;
    ev.events = EPOLLET | EPOLLIN;

    epoll_ctl(epFd, EPOLL_CTL_ADD, server->sockFd, &ev);
    epoll_event evs[256];
    char buf[1024];
    char msg[] = "HTTP/1.1 200 OK\r\n\r\n";
    server->SetBlock(false);
    while (true)
    {
        int count = epoll_wait(epFd, evs, 256, 500);
        if (count < 0)
        {
            break;
        }
        for (int i = 0; i < count; ++i)
        {
            if (evs[i].data.fd == server->sockFd)
            {
                for (;;)
                {
                    auto conn = server->Accept();
                    if (conn.sockFd <= 0) break;
                    ev.data.fd = conn.sockFd;
                    ev.events = EPOLLET | EPOLLIN;
                    epoll_ctl(epFd, EPOLL_CTL_ADD, conn.sockFd, &ev);
                    std::cout << "conn join" << "\n";
                }
            } else
            {
                XTcp client;
                client.sockFd = evs[i].data.fd;
                client.Receive(buf, 1024);
                client.Send(msg, sizeof(msg) + 1);
                epoll_ctl(epFd, EPOLL_CTL_DEL, client.sockFd, &ev);
                client.Close();
            }
        }
    }
}
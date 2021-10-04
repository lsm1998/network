//
// Created by 刘时明 on 2021/10/4.
//

#include "xtcp.h"
#include <sys/select.h>

constexpr int FD_SET_SIZE = 10;

constexpr int MAX_LINE = 128;

void selectServer()
{
    auto *server = new XTcp(8888);
    server->CreateSocket();
    server->Bind();
    server->SetBlock(false);

    fd_set read_fds;
    FD_ZERO(&read_fds);


    timeval _timeval = {};
    _timeval.tv_sec = 1;
    _timeval.tv_usec = 0;

    struct sockaddr_in client_adder{};
    int client_fd;
    char buf[MAX_LINE] = {};
    while (true)
    {
        FD_SET(server->sockFd, &read_fds);
        int n = select(FD_SET_SIZE, &read_fds, nullptr, nullptr, &_timeval);
        if (n == 0)
        {
            std::cout << "empty loop..." << std::endl;
            continue;
        }
        if (FD_ISSET(server->sockFd, &read_fds))
        {
            // 如果服务器fd可用，则为accept
            socklen_t client_adder_len = sizeof(struct sockaddr_in);
            client_fd = accept(server->sockFd, (struct sockaddr *) &client_adder, &client_adder_len);
            if (client_fd == -1)
            {
                printf("accept() error\n");
                return;
            } else
            {
                FD_SET(client_fd, &read_fds);
                printf("%s connect at fd %d\n", inet_ntoa(client_adder.sin_addr), client_fd);
            }
        }
        // 依次查询
        for (int fd = 0; fd < FD_SET_SIZE; fd++)
        {
            if (FD_ISSET(fd, &read_fds))
            {
                // 如果是client活动，进行echo
                int len = read(fd, buf, MAX_LINE);
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
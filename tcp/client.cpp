//
// Created by Administrator on 2021/9/5.
//
#ifdef WIN32
#include <windows.h>
#elif __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <cstring>
#include <thread>
#include <iostream>

constexpr unsigned short PORT = 8080;

constexpr char adder[20] = "127.0.0.1";

int main()
{
    struct sockaddr_in serv_adder = {};
    char buf[1024];
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serv_adder, sizeof(serv_adder));
    serv_adder.sin_family = AF_INET;
    serv_adder.sin_addr.s_addr = inet_addr(adder);
    serv_adder.sin_port = htons(PORT);
    if (connect(sock_fd, (struct sockaddr *) &serv_adder, sizeof(serv_adder)) < 0)
    {
        exit(1);
    }
    while (fgets(buf, 1024, stdin) != nullptr)
    {
        write(sock_fd, buf, strlen(buf));
        ssize_t n = read(sock_fd, buf, 1024);
        if (n == 0)
        {
            printf("the other side has been closed.\n");
            break;
        } else
        {
            printf("收到数据:%s\n", buf);
        }
    }
    close(sock_fd);
    return 0;
}
//
// Created by Administrator on 2021/9/5.
//
#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#include <windows.h>
#define close closesocket
#define socklen_t int
#define ssize_t int
#elif __linux__ || __APPLE__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <cstring>
#include <thread>
#include <iostream>

constexpr unsigned short PORT = 8888;

constexpr char adder[20] = "127.0.0.1";

int main()
{
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    struct sockaddr_in serv_adder = {};
    char buf[1024];
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_adder, 0, sizeof(serv_adder));
    serv_adder.sin_family = AF_INET;
    serv_adder.sin_addr.s_addr = inet_addr(adder);
    serv_adder.sin_port = htons(PORT);
    if (connect(sock_fd, (struct sockaddr *) &serv_adder, sizeof(serv_adder)) < 0)
    {
        exit(1);
    }
    while (fgets(buf, 1024, stdin) != nullptr)
    {
        send(sock_fd, buf, strlen(buf), 0);
        ssize_t n = recv(sock_fd, buf, 1024, 0);
        if (n == 0)
        {
            printf("the other side has been closed.\n");
            break;
        } else
        {
            printf("recv data:%s\n", buf);
        }
    }
    close(sock_fd);
    return 0;
}
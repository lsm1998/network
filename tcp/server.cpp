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

constexpr unsigned short PORT = 5555;

constexpr char adder[20] = "127.0.0.1";

void handlerClient(int clientFd, sockaddr_in &clientAddr, socklen_t socklen)
{
    char buf[1024];
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        ssize_t len = recv(clientFd, buf, sizeof(buf), 0);
        if (len == 0)
        {
            std::cout << "客户端主动退出" << std::endl;
            break;
        }
        std::cout << "收到数据来自" <<
                  inet_ntoa(clientAddr.sin_addr) << ":" <<
                  clientAddr.sin_port << "的数据" <<
                  len << "(bytes)" << std::endl;
        if (strcmp(buf, "exit") == 0)
        {
            break;
        }
        write(clientFd, buf, len);
    }
    close(clientFd);
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket fail");
        return 0;
    }

    sockaddr_in sockAddr = {};
    // 2设置地址信息
    sockAddr.sin_port = htons(PORT);
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(adder);
    if (bind(fd, reinterpret_cast<const sockaddr *>(&sockAddr), sizeof(sockAddr)) != 0)
    {
        perror("bind fail");
        return -1;
    }
    if (listen(fd, 10) < 0)
    {
        perror("listen fail");
        return -1;
    }
    while (true)
    {
        sockaddr_in clientAddr = {};
        socklen_t len = 0;
        int clientFd = accept(fd, reinterpret_cast<sockaddr *>(&clientAddr), &len);
        if (clientFd < 0)
        {
            perror("accept fail");
            break;
        }
        std::thread t(handlerClient, clientFd, std::ref(clientAddr), len);
        t.detach();
    }
    return 0;
}
//
// Created by 180PT73 on 2021/12/10.
//

#ifndef NETWORK_ANET_H
#define NETWORK_ANET_H

#include <unistd.h>
#include <sys/types.h>
#include <iostream>

#define ANET_OK 0
#define ANET_ERR -1

#define CONFIG_BINDADDR_MAX 16

class AnetFd
{
private:
    int fd;

public:
    AnetFd() = default;

    AnetFd(int fd) : fd(fd)
    {};

    ~AnetFd();

    int anetNonBlock();

    int anetBlock();

    int anetCloexec();

    int anetKeepAlive(int interval);

    void anetTcpServer(int port, char *bindaddr, int backlog);

    void anetTcp6Server(int port, char *bindaddr, int backlog);

    int closeFd();

    int fdValue();

    void showInfo();

    void setFd(const int &fd);

    bool operator ==(const int& val);

    bool operator >(const int& val);
};

class socketFds
{
public:
    AnetFd fd[CONFIG_BINDADDR_MAX];
    //int fd[CONFIG_BINDADDR_MAX];
    int count;

    ~socketFds()
    {
        std::cout << "socketFds delete" << std::endl;
    }
};

int anetCloexec(int fd);

#endif //NETWORK_ANET_H

//
// Created by Administrator on 2021/9/6.
//

#ifndef NETWORK_XTCP_H
#define NETWORK_XTCP_H

#ifdef WIN32
#include <windows.h>
#define close closesocket
#pragma comment (lib, "ws2_32.lib")
#elif __linux__ || __APPLE__

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#endif

#include <mutex>
#include <iostream>

constexpr int LISTEN_SIZE = 16;

extern std::once_flag onceFlag;

class XTcp
{
public:
    int sockFd;
private:

private:
    unsigned short port;

    std::string ip;

public:
    XTcp() : XTcp(8080)
    {}

    XTcp(unsigned short port);

    virtual ~XTcp();

    XTcp Accept() const;

    int Close() const;

    int Receive(char buf[], int bufSize) const;

    int Send(const char buf[], int bufSize) const;

    int CreateSocket();

    int Bind() const;

    int Connect(const std::string &ip, unsigned short port, int timeout = 1000);

    int SetBlock(bool block);
};


#endif //NETWORK_XTCP_H

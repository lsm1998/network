//
// Created by Administrator on 2021/9/6.
//

#ifndef NETWORK_XTCP_H
#define NETWORK_XTCP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>


constexpr int LISTEN_SIZE = 16;

class XTcp
{
private:
    int sockFd;

    unsigned short port;

    std::string ip;

public:
    XTcp() : XTcp(8080){}

    XTcp(unsigned short port);

    virtual ~XTcp();

    XTcp Accept() const;

    int Close() const;

    int Receive(char buf[], int bufSize) const;

    int Send(const char buf[], int bufSize) const;

    int CreateSocket();

    int Bind() const;
};


#endif //NETWORK_XTCP_H

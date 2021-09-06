//
// Created by Administrator on 2021/9/6.
//

#include "xtcp.h"

XTcp::XTcp(unsigned short port)
{
#ifdef WIN32
    static bool first = false;
    if(first)
    {

    }
#endif
    this->port = port;
}

int XTcp::CreateSocket()
{
    this->sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockFd < 0)
    {
        perror("CreateSocket fail");
        return this->sockFd;
    }
    return this->sockFd;
}

int XTcp::Bind() const
{
    sockaddr_in sockAddrIn = {};
    sockAddrIn.sin_port = htons(this->port);
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_addr.s_addr = htonl(0);
    socklen_t len = sizeof(sockAddrIn);
    if (bind(this->sockFd, reinterpret_cast<const sockaddr *>(&sockAddrIn), len) < 0)
    {
        perror("bind fail");
        return -2;
    }
    if (listen(this->sockFd, LISTEN_SIZE) < 0)
    {
        perror("listen fail");
        return -1;
    }
    return 0;
}

XTcp XTcp::Accept() const
{
    XTcp tcp = {};
    sockaddr_in clientAddr = {};
    socklen_t len = 0;
    int clientFd = accept(this->sockFd, reinterpret_cast<sockaddr *>(&clientAddr), &len);
    tcp.sockFd = clientFd;
    tcp.port = clientAddr.sin_port;
    tcp.ip = inet_ntoa(clientAddr.sin_addr);
    return tcp;
}

int XTcp::Close() const
{
    return close(this->sockFd);
}

int XTcp::Receive(char *buf, int bufSize) const
{
    return recv(this->sockFd, buf, bufSize, 0);
}

int XTcp::Send(const char *buf, int bufSize) const
{
    return send(this->sockFd, buf, bufSize, 0);
}

XTcp::~XTcp()
{
    this->Close();
}

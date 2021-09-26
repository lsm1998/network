//
// Created by Administrator on 2021/9/6.
//

#include "xtcp.h"

std::once_flag onceFlag;

XTcp::XTcp(unsigned short port)
{
#ifdef WIN32
    std::call_once(onceFlag, []() -> void
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    });
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
#ifdef WIN32
    int len = sizeof(sockAddrIn);
#elif __linux__ || __APPLE__
    socklen_t len = sizeof(sockAddrIn);
#endif
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
#ifdef WIN32
    int len = sizeof(clientAddr);
#elif __linux__ || __APPLE__
    socklen_t len = sizeof(clientAddr);
#endif
    int clientFd = accept(this->sockFd, reinterpret_cast<sockaddr *>(&clientAddr), &len);
    tcp.sockFd = clientFd;
    tcp.port = clientAddr.sin_port;
    tcp.ip = inet_ntoa(clientAddr.sin_addr);
    return tcp;
}

int XTcp::Close() const
{
#ifdef WIN32
    WSACleanup();
#endif
    return close(this->sockFd);
}

int XTcp::Receive(char buf[], int bufSize) const
{
    return recv(this->sockFd, buf, bufSize,0);
}

int XTcp::Send(const char *buf, int bufSize) const
{
    return send(this->sockFd, buf, bufSize, 0);
}

XTcp::~XTcp()
{
    // this->Close();
}

int XTcp::Connect(const std::string &ip, unsigned short port, int timeout)
{
    if (this->sockFd > 0 || timeout < 0)
    {
        return -1;
    }
    this->CreateSocket();
    sockaddr_in sockAddrIn = {};
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_port = htons(port);
    sockAddrIn.sin_addr.s_addr = inet_addr(ip.c_str());

    if (timeout > 0)
    {
        this->SetBlock(false);
    }
    fd_set set = {};
    if (connect(this->sockFd, reinterpret_cast<const sockaddr *>(&sockAddrIn), sizeof(sockAddrIn)) != 0)
    {
        FD_ZERO(&set);
        FD_SET(this->sockFd, &set);
        timeval tm = {};
        tm.tv_sec = 0;
        tm.tv_usec = timeout * 1000;
        if (select(this->sockFd + 1, nullptr, &set, nullptr, &tm) <= 0)
        {
            perror("select fail");
            return -1;
        }
    }
    this->SetBlock(true);
    return 1;
}

int XTcp::SetBlock(bool block)
{
    if (this->sockFd > 0)
    {
        return -1;
    }
#ifdef __WIN32__

#elif __linux__ || __APPLE__
    int flags;
    if ((flags = fcntl(this->sockFd, F_GETFL, 0)) < 0)
    {
        return flags;
    }
    if (block)
    {
        flags = flags & ~O_NONBLOCK;
    } else
    {
        flags |= O_NONBLOCK;
    }
    return fcntl(this->sockFd, F_SETFL, flags);
#endif
}

//
// Created by Administrator on 2021/12/12.
//
#include "anet.h"
#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/stat.h>

static int anetSetBlock(int fd, int non_block)
{
    int flags;
    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        perror("fcntl(F_GETFL) fail");
        return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl(F_SETFL,O_NONBLOCK) fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetSetReuseAddr(int fd)
{
    int yes = 1;
    /* Make sure connection-intensive things like the redis benchmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt SO_REUSEADDR fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog)
{
    if (bind(s, sa, len) == -1)
    {
        perror("bind fail");
        close(s);
        return ANET_ERR;
    }

    if (listen(s, backlog) == -1)
    {
        perror("listen fail");
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetV6Only(int s)
{
    int yes = 1;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt fail");
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}

static int _anetTcpServer(int port, char *bindaddr, int af, int backlog)
{
    int s = -1, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints{}, *servinfo, *p;
    snprintf(_port, 6, "%d", port);
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */
    if (bindaddr && !strcmp("*", bindaddr))
        bindaddr = nullptr;
    if (af == AF_INET6 && bindaddr && !strcmp("::*", bindaddr))
        bindaddr = nullptr;

    if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0)
    {
        perror("getaddrinfo fail");
        return ANET_ERR;
    }
    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (af == AF_INET6 && anetV6Only(s) == ANET_ERR) goto error;
        if (anetSetReuseAddr(s) == ANET_ERR) goto error;

//        sockAddrIn.sin_port = htons(8888);
//        sockAddrIn.sin_family = AF_INET;
//        sockAddrIn.sin_addr.s_addr = htonl(0);
        if (anetListen(s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) s = ANET_ERR;
        goto end;
    }
    if (p == nullptr)
    {
        perror("unable to bind socket fail");
        goto error;
    }

    error:
    if (s != ANET_ERR) close(s);
    s = ANET_ERR;
    end:
    freeaddrinfo(servinfo);
    return s;
}

static int _anetTcpServerV2(int port, int af, int backlog)
{
    int sock_fd = socket(af, SOCK_STREAM, 0);
    sockaddr_in sockAddrIn = {};
    sockAddrIn.sin_port = htons(port);
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_addr.s_addr = htonl(0);
    socklen_t len = sizeof(sockAddrIn);
    if (bind(sock_fd, reinterpret_cast<const sockaddr *>(&sockAddrIn), len) < 0)
    {
        perror("bind fail");
        return -1;
    }
    if (listen(sock_fd, backlog) < 0)
    {
        perror("listen fail");
        return -1;
    }
    return sock_fd;
}

int anetCloexec(int fd)
{
    int r;
    int flags;
    do
    {
        r = fcntl(fd, F_GETFD);
    } while (r == -1 && errno == EINTR);

    if (r == -1 || (r & FD_CLOEXEC))
    {
        return r;
    }
    flags = r | FD_CLOEXEC;
    do
    {
        r = fcntl(fd, F_SETFD, flags);
    } while (r == -1 && errno == EINTR);
    return r;
}

int AnetFd::anetBlock()
{
    return anetSetBlock(fd, 1);
}

int AnetFd::anetNonBlock()
{
    return anetSetBlock(fd, 0);
}

int AnetFd::anetCloexec()
{
    return ::anetCloexec(fd);
}

int AnetFd::anetKeepAlive(int interval)
{
    int val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        perror("setsockopt SO_KEEPALIVE fail");
        return ANET_ERR;
    }
#ifdef __linux__
    /* Default settings are more or less garbage, with the keepalive time
     * set to 7200 by default on Linux. Modify settings to make the feature
     * actually useful. */

    /* Send first probe after interval. */
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0)
    {
        perror("setsockopt TCP_KEEPIDLE fail");
        return ANET_ERR;
    }

    /* Send next probes after the specified interval. Note that we set the
     * delay as interval / 3, as we send three probes before detecting
     * an error (see the next setsockopt call). */
    val = interval / 3;
    if (val == 0) val = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0)
    {
        perror("setsockopt TCP_KEEPINTVL fail");
        return ANET_ERR;
    }

    /* Consider the socket in error state after three we send three ACK
     * probes without getting a reply. */
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0)
    {
        perror("setsockopt TCP_KEEPCNT fail");
        return ANET_ERR;
    }
#else
    ((void) interval); /* Avoid unused var warning for non Linux systems. */
#endif
    return ANET_OK;
}

void AnetFd::anetTcp6Server(int port, char *bindaddr, int backlog)
{
    this->fd = _anetTcpServer(port, bindaddr, AF_INET6, backlog);
}

void AnetFd::anetTcpServer(int port, char *bindaddr, int backlog)
{
    // this->fd = _anetTcpServer(port, bindaddr, AF_INET, backlog);
    this->fd = _anetTcpServerV2(port, AF_INET, backlog);
}

int AnetFd::closeFd()
{
    return close(this->fd);
}

int AnetFd::fdValue()
{
    return this->fd;
}

bool AnetFd::operator==(const int &val)
{
    return this->fd == val;
}

bool AnetFd::operator>(const int &val)
{
    return this->fd > val;
}

void AnetFd::setFd(const int &fd)
{
    this->fd = fd;
}

AnetFd::~AnetFd()
{
    std::cout << "AnetFd delete" << std::endl;
}

void AnetFd::showInfo()
{
    if (this->fd == 0)
    {
        std::cout << "fd未初始化" << std::endl;
    } else
    {
        std::cout << "fd=" << this->fd << std::endl;
    }
}

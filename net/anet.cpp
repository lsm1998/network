//
// Created by 180PT73 on 2021/12/10.
//

#include <iostream>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include "anet.h"

static int anetSetBlock(char *err, int fd, int non_block)
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

int anetNonBlock(char *err, int fd)
{
    return anetSetBlock(err, fd, 1);
}

int anetBlock(char *err, int fd)
{
    return anetSetBlock(err, fd, 0);
}

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int anetKeepAlive(char *err, int fd, int interval)
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
        perror("setsockopt TCP_KEEPINTV fail");
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

static int anetSetTcpNoDelay(char *err, int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        perror("setsockopt TCP_NODELAY fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetEnableTcpNoDelay(char *err, int fd)
{
    return anetSetTcpNoDelay(err, fd, 1);
}

int anetDisableTcpNoDelay(char *err, int fd)
{
    return anetSetTcpNoDelay(err, fd, 0);
}

int anetSetSendBuffer(char *err, int fd, int buffsize)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1)
    {
        perror("setsockopt SO_SNDBUF fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetTcpKeepAlive(char *err, int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt SO_KEEPALIVE fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
 * number of milliseconds, or disable it if the 'ms' argument is zero. */
int anetSendTimeout(char *err, int fd, long long ms)
{
    struct timeval tv{};
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1)
    {
        perror("setsockopt SO_SNDTIMEO fail");
        return ANET_ERR;
    }
    return ANET_OK;
}

/* Set the socket receive timeout (SO_RCVTIMEO socket option) to the specified
 * number of milliseconds, or disable it if the 'ms' argument is zero. */
int anetRecvTimeout(char *err, int fd, long long ms)
{
    struct timeval tv{};

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
    {
        perror("setsockopt SO_RCVTIMEO fail");
        return ANET_ERR;
    }
    return ANET_OK;
}


/* anetGenericResolve() is called by anetResolve() and anetResolveIP() to
 * do the actual work. It resolves the hostname "host" and set the string
 * representation of the IP address into the buffer pointed by "ipbuf".
 *
 * If flags is set to ANET_IP_ONLY the function only resolves hostnames
 * that are actually already IPv4 or IPv6 addresses. This turns the function
 * into a validating / normalizing function. */
int anetGenericResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len, int flags)
{
    struct addrinfo hints{}, *info;
    int rv;

    if (flags & ANET_IP_ONLY) hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, nullptr, &hints, &info)) != 0)
    {
        perror("getaddrinfo fail");
        return ANET_ERR;
    }
    if (info->ai_family == AF_INET)
    {
        auto *sa = (struct sockaddr_in *) info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    } else
    {
        auto *sa = (struct sockaddr_in6 *) info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }
    freeaddrinfo(info);
    return ANET_OK;
}

int anetResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len)
{
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_NONE);
}

int anetResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len)
{
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_IP_ONLY);
}

static int anetSetReuseAddr(char *err, int fd)
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

static int anetCreateSocket(char *err, int domain)
{
    int s;
    if ((s = socket(domain, SOCK_STREAM, 0)) == -1)
    {
        perror("creating socket fail");
        return ANET_ERR;
    }

    /* Make sure connection-intensive things like the redis benchmark
     * will be able to close/open sockets a zillion of times */
    if (anetSetReuseAddr(err, s) == ANET_ERR)
    {
        close(s);
        return ANET_ERR;
    }
    return s;
}

#define ANET_CONNECT_NONE 0
#define ANET_CONNECT_NONBLOCK 1
#define ANET_CONNECT_BE_BINDING 2 /* Best effort binding. */

static int anetTcpGenericConnect(char *err, const char *addr, int port, const char *source_addr, int flags)
{
    int s = ANET_ERR, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints{}, *servinfo, *bservinfo, *p, *b;

    snprintf(portstr, sizeof(portstr), "%d", port);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0)
    {
        perror("getaddrinfo fail");
        return ANET_ERR;
    }
    for (p = servinfo; p != nullptr; p = p->ai_next)
    {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;
        if (anetSetReuseAddr(err, s) == ANET_ERR) goto error;
        if (flags & ANET_CONNECT_NONBLOCK && anetNonBlock(err, s) != ANET_OK)
            goto error;
        if (source_addr)
        {
            int bound = 0;
            /* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
            if ((rv = getaddrinfo(source_addr, nullptr, &hints, &bservinfo)) != 0)
            {
                perror("getaddrinfo fail");
                goto error;
            }
            for (b = bservinfo; b != nullptr; b = b->ai_next)
            {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1)
                {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (!bound)
            {
                perror("bind fail");
                goto error;
            }
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1)
        {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & ANET_CONNECT_NONBLOCK)
                goto end;
            close(s);
            s = ANET_ERR;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
         * have a connected socket. Let's return to the caller. */
        goto end;
    }
    if (p == nullptr)
    {
        perror("creating socket fail");
    }

    error:
    if (s != ANET_ERR)
    {
        close(s);
        s = ANET_ERR;
    }

    end:
    freeaddrinfo(servinfo);

    /* Handle best effort binding: if a binding address was used, but it is
     * not possible to create a socket, try again without a binding address. */
    if (s == ANET_ERR && source_addr && (flags & ANET_CONNECT_BE_BINDING))
    {
        return anetTcpGenericConnect(err, addr, port, nullptr, flags);
    } else
    {
        return s;
    }
}

int anetTcpConnect(char *err, const char *addr, int port)
{
    return anetTcpGenericConnect(err, addr, port, nullptr, ANET_CONNECT_NONE);
}

int anetTcpNonBlockConnect(char *err, const char *addr, int port)
{
    return anetTcpGenericConnect(err, addr, port, nullptr, ANET_CONNECT_NONBLOCK);
}

int anetTcpNonBlockBindConnect(char *err, const char *addr, int port,
                               const char *source_addr)
{
    return anetTcpGenericConnect(err, addr, port, source_addr, ANET_CONNECT_NONBLOCK);
}

int anetTcpNonBlockBestEffortBindConnect(char *err, const char *addr, int port, const char *source_addr)
{
    return anetTcpGenericConnect(err, addr, port, source_addr, ANET_CONNECT_NONBLOCK | ANET_CONNECT_BE_BINDING);
}

int anetUnixGenericConnect(char *err, const char *path, int flags)
{
    int s;
    struct sockaddr_un sa{};

    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR)
        return ANET_ERR;

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (flags & ANET_CONNECT_NONBLOCK)
    {
        if (anetNonBlock(err, s) != ANET_OK)
        {
            close(s);
            return ANET_ERR;
        }
    }
    if (connect(s, (struct sockaddr *) &sa, sizeof(sa)) == -1)
    {
        if (errno == EINPROGRESS &&
            flags & ANET_CONNECT_NONBLOCK)
            return s;
        perror("connect fail");
        close(s);
        return ANET_ERR;
    }
    return s;
}

int anetUnixConnect(char *err, const char *path)
{
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONE);
}

int anetUnixNonBlockConnect(char *err, const char *path)
{
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONBLOCK);
}

/* Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered) */
int anetRead(int fd, char *buf, int count)
{
    ssize_t nread, totlen = 0;
    while (totlen != count)
    {
        nread = read(fd, buf, count - totlen);
        if (nread == 0) return totlen;
        if (nread == -1) return -1;
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

/* Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) */
int anetWrite(int fd, char *buf, int count)
{
    ssize_t nwritten, totlen = 0;
    while (totlen != count)
    {
        nwritten = write(fd, buf, count - totlen);
        if (nwritten == 0) return totlen;
        if (nwritten == -1) return -1;
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog)
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

static int anetV6Only(char *err, int s)
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

static int _anetTcpServer(char *err, int port, char *bindaddr, int af, int backlog)
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

        if (af == AF_INET6 && anetV6Only(err, s) == ANET_ERR) goto error;
        if (anetSetReuseAddr(err, s) == ANET_ERR) goto error;
        if (anetListen(err, s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) s = ANET_ERR;
        goto end;
    }
    if (p == nullptr)
    {
        perror("unable to bind socket fail");
        goto error;
    }

    error:
    if (s != -1) close(s);
    s = ANET_ERR;
    end:
    freeaddrinfo(servinfo);
    return s;
}

int anetTcpServer(char *err, int port, char *bindaddr, int backlog)
{
    return _anetTcpServer(err, port, bindaddr, AF_INET, backlog);
}

int anetTcp6Server(char *err, int port, char *bindaddr, int backlog)
{
    return _anetTcpServer(err, port, bindaddr, AF_INET6, backlog);
}

int anetUnixServer(char *err, char *path, mode_t perm, int backlog)
{
    int s;
    struct sockaddr_un sa{};
    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR)
    {
        return ANET_ERR;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (anetListen(err, s, (struct sockaddr *) &sa, sizeof(sa), backlog) == ANET_ERR)
        return ANET_ERR;
    if (perm)
    {
        chmod(sa.sun_path, perm);
    }
    return s;
}

static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len)
{
    int fd;
    while (true)
    {
        fd = accept(s, sa, len);
        if (fd == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                perror("accept fail");
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}

int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port)
{
    int fd;
    struct sockaddr_storage sa{};
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err, s, (struct sockaddr *) &sa, &salen)) == -1)
        return ANET_ERR;

    if (sa.ss_family == AF_INET)
    {
        auto *s = (struct sockaddr_in *) &sa;
        if (ip) inet_ntop(AF_INET, (void *) &(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    } else
    {
        auto *s = (struct sockaddr_in6 *) &sa;
        if (ip) inet_ntop(AF_INET6, (void *) &(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    return fd;
}

int anetUnixAccept(char *err, int s)
{
    int fd;
    struct sockaddr_un sa{};
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err, s, (struct sockaddr *) &sa, &salen)) == -1)
        return ANET_ERR;

    return fd;
}

int anetFdToString(int fd, char *ip, size_t ip_len, int *port, int fd_to_str_type)
{
    struct sockaddr_storage sa{};
    socklen_t salen = sizeof(sa);

    if (fd_to_str_type == FD_TO_PEER_NAME)
    {
        if (getpeername(fd, (struct sockaddr *) &sa, &salen) == -1) goto error;
    } else
    {
        if (getsockname(fd, (struct sockaddr *) &sa, &salen) == -1) goto error;
    }
    if (ip_len == 0) goto error;

    if (sa.ss_family == AF_INET)
    {
        auto *s = (struct sockaddr_in *) &sa;
        if (ip) inet_ntop(AF_INET, (void *) &(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    } else if (sa.ss_family == AF_INET6)
    {
        auto *s = (struct sockaddr_in6 *) &sa;
        if (ip) inet_ntop(AF_INET6, (void *) &(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    } else if (sa.ss_family == AF_UNIX)
    {
        if (ip) snprintf(ip, ip_len, "/unixsocket");
        if (port) *port = 0;
    } else
    {
        goto error;
    }
    return 0;

    error:
    if (ip)
    {
        if (ip_len >= 2)
        {
            ip[0] = '?';
            ip[1] = '\0';
        } else if (ip_len == 1)
        {
            ip[0] = '\0';
        }
    }
    if (port) *port = 0;
    return -1;
}

/* Format an IP,port pair into something easy to parse. If IP is IPv6
 * (matches for ":"), the ip is surrounded by []. IP and port are just
 * separated by colons. This the standard to display addresses within Redis. */
int anetFormatAddr(char *buf, size_t buf_len, char *ip, int port)
{
    return snprintf(buf, buf_len, strchr(ip, ':') ? "[%s]:%d" : "%s:%d", ip, port);
}

/* Like anetFormatAddr() but extract ip and port from the socket's peer/sockname. */
int anetFormatFdAddr(int fd, char *buf, size_t buf_len, int fd_to_str_type)
{
    char ip[INET6_ADDRSTRLEN];
    int port;

    anetFdToString(fd, ip, sizeof(ip), &port, fd_to_str_type);
    return anetFormatAddr(buf, buf_len, ip, port);
}
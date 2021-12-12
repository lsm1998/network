//
// Created by 180PT73 on 2021/12/10.
//

#ifndef NETWORK_ANET_H
#define NETWORK_ANET_H

#include <unistd.h>
#include <sys/types.h>

/* 是否有epoll */
#ifdef __linux__
#define HAVE_EPOLL 1
#endif

/* 是否有kqueue */
#if (defined(__APPLE__)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define HAVE_KQUEUE 1
#endif

/* 是否有select */
#if (!HAVE_EPOLL && !HAVE_KQUEUE)
#define HAVE_SELECT
#endif

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

/* Flags used with certain functions. */
#define ANET_NONE 0
#define ANET_IP_ONLY (1<<0)

/* FD to address string conversion types */
#define FD_TO_PEER_NAME 0
#define FD_TO_SOCK_NAME 1

int anetTcpConnect(char *err, const char *addr, int port);

int anetTcpNonBlockConnect(char *err, const char *addr, int port);

int anetTcpNonBlockBindConnect(char *err, const char *addr, int port, const char *source_addr);

int anetTcpNonBlockBestEffortBindConnect(char *err, const char *addr, int port, const char *source_addr);

int anetUnixConnect(char *err, const char *path);

int anetUnixNonBlockConnect(char *err, const char *path);

int anetRead(int fd, char *buf, int count);

int anetResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);

int anetResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);

int anetTcpServer(char *err, int port, char *bindaddr, int backlog);

int anetTcp6Server(char *err, int port, char *bindaddr, int backlog);

int anetUnixServer(char *err, char *path, mode_t perm, int backlog);

int anetTcpAccept(char *err, int serversock, char *ip, size_t ip_len, int *port);

int anetUnixAccept(char *err, int serversock);

int anetWrite(int fd, char *buf, int count);

int anetNonBlock(char *err, int fd);

int anetBlock(char *err, int fd);

int anetCloexec(int fd);

int anetEnableTcpNoDelay(char *err, int fd);

int anetDisableTcpNoDelay(char *err, int fd);

int anetTcpKeepAlive(char *err, int fd);

int anetSendTimeout(char *err, int fd, long long ms);

int anetRecvTimeout(char *err, int fd, long long ms);

int anetFdToString(int fd, char *ip, size_t ip_len, int *port, int fd_to_str_type);

int anetKeepAlive(char *err, int fd, int interval);

int anetFormatAddr(char *fmt, size_t fmt_len, char *ip, int port);

int anetFormatFdAddr(int fd, char *buf, size_t buf_len, int fd_to_str_type);

#endif //NETWORK_ANET_H

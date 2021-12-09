//
// Created by Administrator on 2021/12/7.
//

#ifndef NETWORK_NET_H
#define NETWORK_NET_H

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

#endif //NETWORK_NET_H

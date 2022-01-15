//
// Created by 刘时明 on 2022/1/15.
//

#ifndef NETWORK_AE_H
#define NETWORK_AE_H

#include <string>
#include "anet.h"

#define AE_NONE 0       /* No events registered. */
#define AE_READABLE 1   /* Fire when descriptor is readable. */
#define AE_WRITABLE 2   /* Fire when descriptor is writable. */

#define AE_FILE_EVENTS (1<<0)
#define AE_TIME_EVENTS (1<<1)
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_CALL_BEFORE_SLEEP (1<<3)
#define AE_CALL_AFTER_SLEEP (1<<4)

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

using String = std::string;

class aeEventLoop;

class aeFiredEvent
{
public:
    int fd;
    int mask;
};

class aeFileEvent
{
    /* one of AE_(READABLE|WRITABLE|BARRIER) */
public:
    int mask{};

    virtual void rfileProc(aeEventLoop *eventLoop, int fd, void *clientData, int mask) = 0;

    virtual void wfileProc(aeEventLoop *eventLoop, int fd, void *clientData, int mask) = 0;

    void *clientData{};
};

class aeEventLoop
{
public:
    int stop{};

    int setsize{}; /* max number of file descriptors tracked */

    int maxfd{};

    aeFileEvent *event{};

    aeFiredEvent *fired{};

    void *apidata{}; /* This is used for polling API specific data */

    int flags{};

    /**
     * 创建
     * @return
     */
    virtual int aeApiCreate() = 0;

    /**
     * 扩容
     * @param setsize
     * @return
     */
    virtual int aeApiResize(int setsize) = 0;

    /**
     * 回收
     */
    virtual void aeApiFree() = 0;

    /**
     * 添加事件
     * @param fd
     * @param mask
     * @return
     */
    virtual int aeApiAddEvent(int fd, int mask) = 0;

    /**
     * 删除事件
     * @param fd
     * @param mask
     */
    virtual void aeApiDelEvent(int fd, int mask) = 0;

    /**
     * poll一次
     * @param tvp
     * @return
     */
    virtual int aeApiPoll(struct timeval *tvp) = 0;

    /**
     * 事件处理名称
     * @return
     */
    virtual String aeApiName() & = 0;

    int aeProcessEvents(int flags);

    long msUntilEarliestTimer();

    int processTimeEvents();

    void aeDeleteFileEvent(int fd, int mask);

    int aeCreateFileEvent(int fd, int mask, void *clientData, aeFileEvent *fileEvent);
};

aeEventLoop *createAeEventLoop(int setsize, aeFileEvent *event);

#endif //NETWORK_AE_H

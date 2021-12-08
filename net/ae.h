//
// Created by Administrator on 2021/12/7.
//

#ifndef NETWORK_AE_H
#define NETWORK_AE_H

#include <iostream>

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2
#define AE_BARRIER 4

constexpr int AE_FILE_EVENTS = 1 << 0;

constexpr int AE_TIME_EVENTS = 1 << 1;

constexpr int AE_ALL_EVENTS = AE_FILE_EVENTS | AE_TIME_EVENTS;

constexpr int AE_CALL_BEFORE_SLEEP = 1 << 3;

constexpr int AE_CALL_AFTER_SLEEP = 1 << 4;

constexpr int AE_DONT_WAIT = 1 << 2;

struct aeEventLoop;

/* Types and data structures */
typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);

typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);

typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

/* Time event structure */
typedef struct aeTimeEvent
{
    long long id; /* time event identifier. */
    aeTimeProc *timeProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeTimeEvent *prev;
    struct aeTimeEvent *next;
    int refcount; /* refcount to prevent timer events from being
  		   * freed in recursive time event calls. */
} aeTimeEvent;

/* A fired event */
typedef struct aeFiredEvent
{
    int fd;
    int mask;
} aeFiredEvent;

/* File event structure */
typedef struct aeFileEvent
{
    int mask; /* one of AE_(READABLE|WRITABLE|BARRIER) */
    aeFileProc *rfileProc;
    aeFileProc *wfileProc;
    void *clientData;
} aeFileEvent;

typedef struct aeEventLoop
{
    int maxfd;   /* highest file descriptor currently registered */
    int setsize; /* max number of file descriptors tracked */
    long long timeEventNextId;
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    aeTimeEvent *timeEventHead;
    int stop;
    void *apidata; /* This is used for polling API specific data */
    aeBeforeSleepProc *beforesleep;
    aeBeforeSleepProc *aftersleep;
    int flags;
} aeEventLoop;

/* Prototypes */
aeEventLoop *aeCreateEventLoop(int setsize);

int aeProcessEvents(aeEventLoop *eventLoop, int flags);

void aeMain(aeEventLoop *eventLoop);

#endif //NETWORK_AE_H

//
// Created by Administrator on 2021/12/7.
//
#include "ae.h"
#include <sys/epoll.h>
#include <zconf.h>

typedef struct aeApiState
{
    int epfd;
    struct epoll_event *events;
} aeApiState;

static int aeApiCreate(aeEventLoop *eventLoop)
{
    auto *state = new aeApiState();
    if (!state)
    {
        return -1;
    }
    state->events = new epoll_event[eventLoop->setsize];

    if (!state->events)
    {
        delete state;
        return -1;
    }
    state->epfd = epoll_create(1024); /* 1024 is just a hint for the kernel */
    if (state->epfd == -1)
    {
        delete[] state->events;
        delete state;
        return -1;
    }
    close(state->epfd);
    eventLoop->apidata = state;
    return 0;
}

static int aeApiPoll(aeEventLoop *eventLoop, struct timeval *tvp)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    int retval, numevents = 0;

    retval = epoll_wait(state->epfd, state->events, eventLoop->setsize,
                        tvp ? (tvp->tv_sec * 1000 + (tvp->tv_usec + 999) / 1000) : -1);
    if (retval > 0)
    {
        int j;
        numevents = retval;
        for (j = 0; j < numevents; j++)
        {
            int mask = 0;
            struct epoll_event *e = state->events + j;

            if (e->events & EPOLLIN) mask |= AE_READABLE;
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
            if (e->events & EPOLLERR) mask |= AE_WRITABLE | AE_READABLE;
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE | AE_READABLE;
            eventLoop->fired[j].fd = e->data.fd;
            eventLoop->fired[j].mask = mask;
        }
    } else if (retval == -1 && errno != EINTR)
    {
        // panic("aeApiPoll: epoll_wait, %s", strerror(errno));
    }
    return numevents;
}

static int aeApiResize(aeEventLoop *eventLoop, int setsize)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    state->events = new epoll_event[setsize];
    return 0;
}

static void aeApiFree(aeEventLoop *eventLoop)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    close(state->epfd);
    delete[] state->events;
    delete state;
}

static int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    struct epoll_event ee = {0}; /* avoid valgrind warning */
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    int op = eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    ee.events = 0;
    mask |= eventLoop->events[fd].mask; /* Merge old events */
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.fd = fd;
    if (epoll_ctl(state->epfd, op, fd, &ee) == -1) return -1;
    return 0;
}
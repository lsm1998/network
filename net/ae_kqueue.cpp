//
// Created by 刘时明 on 2021/12/9.
//
#include "anet.h"

#ifdef HAVE_KQUEUE

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include "ae.h"

typedef struct aeApiState
{
    int kq_fd;
    struct kevent *events;
} aeApiState;

static int aeApiCreate(aeEventLoop *eventLoop)
{
    auto *state = new aeApiState();
    state->events = new struct kevent[eventLoop->setsize];

    state->kq_fd = kqueue();
    if (state->kq_fd == -1)
    {
        delete[] state->events;
        delete state;
        return -1;
    }
    eventLoop->apidata = state;
    return 0;
}

static int aeApiResize(aeEventLoop *eventLoop, int setsize)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    state->events = new struct kevent[setsize];
    return 0;
}

static void aeApiFree(aeEventLoop *eventLoop)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    close(state->kq_fd);
    delete[] state->events;
    delete state;
}

static int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    struct kevent ke{};

    if (mask & AE_READABLE)
    {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
        if (kevent(state->kq_fd, &ke, 1, nullptr, 0, nullptr) == -1) return -1;
    }
    if (mask & AE_WRITABLE)
    {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, nullptr);
        if (kevent(state->kq_fd, &ke, 1, nullptr, 0, nullptr) == -1) return -1;
    }
    return 0;
}

static void aeApiDelEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    struct kevent ke{};

    if (mask & AE_READABLE)
    {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        kevent(state->kq_fd, &ke, 1, nullptr, 0, nullptr);
    }
    if (mask & AE_WRITABLE)
    {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        kevent(state->kq_fd, &ke, 1, nullptr, 0, nullptr);
    }
}

static int aeApiPoll(aeEventLoop *eventLoop, struct timeval *tvp)
{
    auto *state = static_cast<aeApiState *>(eventLoop->apidata);
    int retval, numevents = 0;

    if (tvp != nullptr)
    {
        struct timespec timeout{};
        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;
        retval = kevent(state->kq_fd, nullptr, 0, state->events, eventLoop->setsize, &timeout);
    } else
    {
        retval = kevent(state->kq_fd, nullptr, 0, state->events, eventLoop->setsize, nullptr);
    }

    if (retval > 0)
    {
        int j;
        numevents = retval;
        for (j = 0; j < numevents; j++)
        {
            int mask = 0;
            struct kevent *e = state->events + j;
            if (e->filter == EVFILT_READ) mask |= AE_READABLE;
            if (e->filter == EVFILT_WRITE) mask |= AE_WRITABLE;
            eventLoop->fired[j].fd = e->ident;
            eventLoop->fired[j].mask = mask;
        }
    }
    return numevents;
}

static char *aeApiName()
{
    return "kqueue";
}

#endif
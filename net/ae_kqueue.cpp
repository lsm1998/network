//
// Created by 刘时明 on 2021/12/9.
//
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

static char *aeApiName()
{
    return "kqueue";
}
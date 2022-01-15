//
// Created by 刘时明 on 2022/1/15.
//
#include "ae.h"

#ifdef HAVE_EPOLL
#include "ae_epoll.h"
#include "poll.h"
#elif HAVE_KQUEUE
#include "ae_kqueue.h"
#elif HAVE_SELECT
#include "ae_select.h"
#endif

aeEventLoop *createAeEventLoop(int setsize, aeFileEvent *event)
{
    aeEventLoop *eventLoop = nullptr;
#ifdef HAVE_EPOLL
    eventLoop = new aeEventLoopEpoll();
#elif HAVE_KQUEUE
    eventLoop = new aeEventLoopKqueue();
#elif HAVE_SELECT
    return nullptr;
#else
    return nullptr;
#endif
    if (eventLoop)
    {
        eventLoop->maxfd = -1;
        eventLoop->setsize = setsize;
        eventLoop->event = reinterpret_cast<aeFileEvent *>(event);
        eventLoop->fired = new aeFiredEvent();
        eventLoop->flags = 0;
    }
    return eventLoop;
}
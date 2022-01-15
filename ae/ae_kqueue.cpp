//
// Created by 刘时明 on 2022/1/15.
//
#include "ae_kqueue.h"

#ifdef HAVE_KQUEUE

#include <sys/event.h>

class aeApiState
{
public:
    int kq_fd;
    struct kevent *events;
    char *eventsMask;
};

#define EVENT_MASK_MALLOC_SIZE(sz) (((sz) + 3) / 4)
#define EVENT_MASK_OFFSET(fd) ((fd) % 4 * 2)
#define EVENT_MASK_ENCODE(fd, mask) (((mask) & 0x3) << EVENT_MASK_OFFSET(fd))


static inline void addEventMask(char *eventsMask, int fd, int mask)
{
    eventsMask[fd / 4] |= EVENT_MASK_ENCODE(fd, mask);
}

static inline int getEventMask(const char *eventsMask, int fd)
{
    return (eventsMask[fd / 4] >> EVENT_MASK_OFFSET(fd)) & 0x3;
}

static inline void resetEventMask(char *eventsMask, int fd)
{
    eventsMask[fd / 4] &= ~EVENT_MASK_ENCODE(fd, 0x3);
}

int aeEventLoopKqueue::aeApiCreate()
{
    auto *state = new aeApiState();
    state->events = new struct kevent[this->setsize];
    state->kq_fd = kqueue();
    if (state->kq_fd == -1)
    {
        delete[] state->events;
        delete state;
        return -1;
    }
    anetCloexec(state->kq_fd);
    state->eventsMask = static_cast<char *>(malloc(EVENT_MASK_MALLOC_SIZE(this->setsize)));
    memset(state->eventsMask, 0, EVENT_MASK_MALLOC_SIZE(this->setsize));
    this->apidata = state;
    return 0;
}

int aeEventLoopKqueue::aeApiResize(int setsize)
{
    auto *state = static_cast<aeApiState *>(this->apidata);
    state->events = new struct kevent[setsize];
    free(state->eventsMask);
    state->eventsMask = static_cast<char *>(malloc(EVENT_MASK_MALLOC_SIZE(setsize)));
    memset(state->eventsMask, 0, EVENT_MASK_MALLOC_SIZE(setsize));
    return 0;
}

void aeEventLoopKqueue::aeApiFree()
{
    auto *state = static_cast<aeApiState *>(this->apidata);
    close(state->kq_fd);
    delete[] state->events;
    delete state->eventsMask;
    delete state;
}

int aeEventLoopKqueue::aeApiAddEvent(int fd, int mask)
{
    auto *state = static_cast<aeApiState *>(this->apidata);
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

void aeEventLoopKqueue::aeApiDelEvent(int fd, int mask)
{
    auto *state = static_cast<aeApiState *>(this->apidata);
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

int aeEventLoopKqueue::aeApiPoll(struct timeval *tvp)
{
    auto *state = static_cast<aeApiState *>(this->apidata);
    int retval, numevents = 0;

    if (tvp != nullptr)
    {
        struct timespec timeout{};
        timeout.tv_sec = tvp->tv_sec;
        timeout.tv_nsec = tvp->tv_usec * 1000;
        retval = kevent(state->kq_fd, nullptr, 0, state->events, this->setsize, &timeout);
    } else
    {
        retval = kevent(state->kq_fd, nullptr, 0, state->events, this->setsize, nullptr);
    }
    if (retval > 0)
    {
        int j;
        for (j = 0; j < retval; j++)
        {
            struct kevent *e = state->events + j;
            int fd = e->ident;
            int mask = 0;

            if (e->filter == EVFILT_READ) mask = AE_READABLE;
            else if (e->filter == EVFILT_WRITE) mask = AE_WRITABLE;
            addEventMask(state->eventsMask, fd, mask);
        }
        numevents = 0;
        for (j = 0; j < retval; j++)
        {
            struct kevent *e = state->events + j;
            int fd = e->ident;
            int mask = getEventMask(state->eventsMask, fd);

            if (mask)
            {
                this->fired[numevents].fd = fd;
                this->fired[numevents].mask = mask;
                resetEventMask(state->eventsMask, fd);
                numevents++;
            }
        }
    }
    return numevents;
}

String aeEventLoopKqueue::aeApiName() &
{
    return "kqueue";
}

#endif
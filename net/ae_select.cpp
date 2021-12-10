//
// Created by Administrator on 2021/12/8.
//
#include "anet.h"
#ifdef HAVE_SELECT

typedef struct aeApiState
{
    int epfd;
    struct epoll_event *events;
} aeApiState;

#endif
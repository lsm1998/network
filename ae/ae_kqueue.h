//
// Created by 刘时明 on 2022/1/15.
//

#ifndef NETWORK_AE_KQUEUE_H
#define NETWORK_AE_KQUEUE_H

#include "ae.h"

#ifdef HAVE_KQUEUE

class aeEventLoopKqueue : public aeEventLoop
{
public:
    int aeApiCreate() override;

    int aeApiResize(int setsize) override;

    void aeApiFree() override;

    int aeApiAddEvent(int fd, int mask) override;

    void aeApiDelEvent(int fd, int mask) override;

    int aeApiPoll(struct timeval *tvp) override;

    String aeApiName() & override;
};

#endif

#endif //NETWORK_AE_KQUEUE_H

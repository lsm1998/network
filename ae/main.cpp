//
// Created by 刘时明 on 2022/1/15.
//
#include "ae.h"
#include <iostream>

class RedisFileEvent : public aeFileEvent
{
public:
    void rfileProc(aeEventLoop *eventLoop, int fd, void *clientData, int mask) override
    {
        std::cout << "rfileProc=" << clientData << std::endl;
    }

    void wfileProc(aeEventLoop *eventLoop, int fd, void *clientData, int mask) override
    {
        std::cout << "wfileProc=" << clientData << std::endl;
    }
};

int main()
{
    auto *redisFileEvent = new RedisFileEvent();
    redisFileEvent->mask = AE_NONE;
    auto el = createAeEventLoop(1024, redisFileEvent);

    std::cout << "当前事件类型 [" << el->aeApiName() << "]" << std::endl;
    int flags = AE_ALL_EVENTS | AE_CALL_BEFORE_SLEEP | AE_CALL_AFTER_SLEEP;
    while (!el->stop)
    {
        el->aeProcessEvents(flags);
    }
    return 0;
}
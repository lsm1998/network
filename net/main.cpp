//
// Created by Administrator on 2021/12/7.
//

#include "main.h"

int main()
{
    char neterr[1024];
    mode_t m{};
    int sofd = anetUnixServer(neterr, "/opt/sock", m, 1);
    if (sofd == ANET_ERR)
    {
        printf("Opening Unix socket: %s \n", neterr);
        exit(1);
    }
    anetNonBlock(nullptr, sofd);
    auto eventLoop = aeCreateEventLoop(1024);
    aeMain(eventLoop);
}
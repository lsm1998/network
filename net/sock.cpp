//
// Created by 刘时明 on 2021/12/11.
//

#include <cstring>
#include "anet.h"
#include <iostream>

int listenToPort(int port, int *fds, int *count, char **bindaddr, int bindaddr_count, int tcp_backlog)
{
    int j;
    char *default_bindaddr[2] = {"*", "-::*"};

    /* Force binding of 0.0.0.0 if no bind address is specified. */
    if (bindaddr_count == 0)
    {
        bindaddr_count = 2;
        bindaddr = default_bindaddr;
    }

    char neterr[1024];
    for (j = 0; j < bindaddr_count; j++)
    {
        char *addr = bindaddr[j];
        int optional = *addr == '-';
        if (optional) addr++;
        if (strchr(addr, ':'))
        {
            /* Bind IPv6 address. */
            fds[*count] = anetTcp6Server(neterr, port, addr, tcp_backlog);
        } else
        {
            /* Bind IPv4 address. */
            fds[*count] = anetTcpServer(neterr, port, addr, tcp_backlog);
        }
        if (fds[*count] == ANET_ERR)
        {
            perror("Could not create server TCP listening socket fail");
            if (errno == EADDRNOTAVAIL && optional)
                continue;
            if (errno == ENOPROTOOPT || errno == EPROTONOSUPPORT ||
                errno == ESOCKTNOSUPPORT || errno == EPFNOSUPPORT ||
                errno == EAFNOSUPPORT)
                continue;
            return -1;
        }
        anetNonBlock(nullptr, fds[*count]);
        (*count)++;
    }
    return 0;
}


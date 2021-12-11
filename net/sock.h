//
// Created by 刘时明 on 2021/12/11.
//

#ifndef NETWORK_SOCK_H
#define NETWORK_SOCK_H

int listenToPort(int port, int *fds, int *count, char **bindaddr, int bindaddr_count, int tcp_backlog);

#endif //NETWORK_SOCK_H

//
// Created by Administrator on 2021/10/28.
//

#ifndef NETWORK_SETPROCTITLE_H
#define NETWORK_SETPROCTITLE_H

#include <unistd.h>
#include <global.h>

void init_set_proc_title();

void set_proc_title(const char *title);

#endif //NETWORK_SETPROCTITLE_H

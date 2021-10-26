//
// Created by Administrator on 2021/10/24.
//

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <csignal>

pid_t nginx_pid;

pid_t nginx_parent;

size_t g_argv_need_mem;

sig_atomic_t nginx_reap;

int nginx_process;

size_t g_env_need_mem;

int g_os_argc;

char **g_os_argv;

#endif //NETWORK_COMMON_H

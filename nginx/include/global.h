//
// Created by 刘时明 on 2020/12/13.
//

#ifndef NGINX_GLOBAL_H
#define NGINX_GLOBAL_H

#include <csignal>
#include <unistd.h>
#include <cstring>

// 版本号
constexpr const char *VERSION = "1.0.0";

// 作者
constexpr const char *AUTHOR = "lsm1998";

// 配置文件名称
constexpr const char *CONFIG_PATH = "nginx.conf";

// 默认日志存放路径和文件名
constexpr const char *DEFAULT_LOG_PATH = "error.log";

// master进程，管理进程
constexpr const u_int PROCESS_MASTER = 0;

// worker进程，工作进程
constexpr const u_int PROCESS_WORKER = 1;

// 最大的32位无符号数
constexpr const uint32_t NGX_MAX_UINT32_VALUE = 0xffffffff;

// u_char
typedef unsigned char u_char;

// u_int
typedef unsigned int u_int;

// 进程pid
extern pid_t nginx_pid;

// 父进程pid
extern pid_t nginx_parent;

// 指向env环境变量的内存
extern char *gp_env_mem;

// argv参数所需要的内存大小
extern size_t g_argv_need_mem;

// 环境变量所占内存大小
extern size_t g_env_need_mem;

// 参数个数
extern int g_os_argc;

// 参数
extern char **g_os_argv;

// 进程类型
extern int nginx_process;

// 标记子进程状态变化
extern sig_atomic_t nginx_reap;

// 标志程序退出,0不退出1，退出
extern int g_stop_event;

#ifdef __linux__
#define HAVE_EPOLL 1
#endif

#if (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define HAVE_KQUEUE 1
#endif

#endif
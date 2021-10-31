//
// Created by 刘时明 on 2020/12/13.
//

#ifndef NGINX_GLOBAL_H
#define NGINX_GLOBAL_H

#include <csignal>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>

// 版本号
constexpr const char *VERSION = "1.0.0";

// 作者
constexpr const char *AUTHOR = "lsm1998";

// 配置文件名称
constexpr const char *CONFIG_PATH = "nginx.conf";

// 默认日志存放路径和文件名
constexpr const char *DEFAULT_LOG_PATH = "error.log";

//
constexpr const int NGX_MAX_ERROR_STR = 2048;

// master进程，管理进程
constexpr const u_int PROCESS_MASTER = 0;

// worker进程，工作进程
constexpr const u_int PROCESS_WORKER = 1;

// 最大的32位无符号数
constexpr const u_int32_t NGX_MAX_UINT32_VALUE = 0xffffffff;

// 最大的int64_t
constexpr const int64_t NGX_INT64_LEN = sizeof("-9223372036854775808") - 1;

// 已完成连接队列，nginx 511
constexpr const int NGX_LISTEN_BACKLOG  =511;

// epoll_wait一次最多事件数量
constexpr const int NGX_MAX_EVENTS = 512;

constexpr const char* NGX_ERROR_LOG_PATH = "error.log";

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

extern size_t g_argvneedmem;

extern size_t g_envneedmem;

extern char *gp_envmem;

extern char **g_os_env;

//和运行日志相关
typedef struct
{
    int log_level;   //日志级别 或者日志类型，ngx_macro.h里分0-8共9个级别
    int fd;          //日志文件描述符

} ngx_log_t;

extern ngx_log_t ngx_log;

#ifdef __linux__
#define HAVE_EPOLL 1
#endif

#if (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
#define HAVE_KQUEUE 1
#endif

#endif
//
// Created by Administrator on 2021/10/24.
//

#ifndef NETWORK_LOG_H
#define NETWORK_LOG_H

#include <iostream>
#include <cstdarg>
#include <global.h>
#include <log_printf.h>
#include <sys/time.h>  //gettimeofday
#include <config.h>
#include <fcntl.h>

// 控制台错误【stderr】
constexpr const char NGX_LOG_STDERR = 0;
//紧急 【emerg】
constexpr const char NGX_LOG_EMERG = 1;
//警戒 【alert】
constexpr const char NGX_LOG_ALERT = 2;
//严重 【crit】
constexpr const char NGX_LOG_CRIT = 3;
//错误 【error】：属于常用级别
constexpr const char NGX_LOG_ERR = 4;
//警告 【warn】：属于常用级别
constexpr const char NGX_LOG_WARN = 5;
//注意 【notice】
constexpr const char NGX_LOG_NOTICE = 6;
//信息 【info】
constexpr const char NGX_LOG_INFO = 7;
//调试 【debug】：最低级别
constexpr const char NGX_LOG_DEBUG = 8;

static u_char err_levels[][20] =
        {
                {"stderr"},    //0：控制台错误
                {"emerg"},     //1：紧急
                {"alert"},     //2：警戒
                {"crit"},      //3：严重
                {"error"},     //4：错误
                {"warn"},      //5：警告
                {"notice"},    //6：注意
                {"info"},      //7：信息
                {"debug"}      //8：调试
        };

inline void DEBUG();

inline void ERROR();

inline void INFO();

inline u_char *ngx_cpymem(void *dst, const void *src, size_t n);

inline void log_stderr(int err, const char *fmt, ...);

inline u_char *ngx_log_errno(unsigned char *buf, const unsigned char *last, int err);

void ngx_log_error_core(int level, int err, const char *fmt, ...);

void ngx_log_init();

#endif //NETWORK_LOG_H

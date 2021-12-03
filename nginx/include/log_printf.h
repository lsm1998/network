//
// Created by Administrator on 2021/10/28.
//

#ifndef NETWORK_PRINTF_H
#define NETWORK_PRINTF_H

#include <iostream>
#include <cstdarg>
#include <global.h>

u_char *slprintf(unsigned char *buf, u_char *last, const char *fmt, ...);

u_char *snprintf(unsigned char *buf, size_t max, const char *fmt, ...);

u_char *vslprintf(u_char *buf, const u_char *last, const char *fmt, va_list args);

static u_char *
sprintf_num(u_char *buf, const u_char *last, uint64_t ui64, u_char zero, uintptr_t hexadecimal, uintptr_t width);

#endif //NETWORK_PRINTF_H

//
// Created by Administrator on 2021/11/2.
//

#include "icmp.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
uint16_t iCMPCheckSum
*/
uint16_t calculateICMPCheckSum(uint16_t *icmp)
{
    int size = calculateICMPSize((void *) icmp);
    uint32_t checkSum = 0;
    int decrementValue = sizeof(uint16_t);
    //16位为单位数字相加
    while (size > 1)
    {
        checkSum += *icmp++;     // 对传入的数据以uint16_t方式解析
        size -= decrementValue;
    }
    //长度奇数情况 size == 1
    if (size)
    {
        checkSum += *(uint8_t *) icmp;
    }
    //高位有进位，进位到低位，下面两行代码保证了高16位为0。
    while (checkSum >> 16)
    {
        checkSum = (checkSum >> 16) + (checkSum & 0xffff);
    }
    //最后按位取反
    return (uint16_t) (~checkSum); // ~是按位取反
}

/**
uint16_t validateICMPCheckSum
 void *icmp 指针类型转换后++不是以转换后字节数累加的还是加1；(有待分析，奇怪)
*/
bool validateICMPCheckSum(uint16_t *icmp)
{
    int size = calculateICMPSize((void *) icmp);
    uint32_t checkSum = 0;
    int decrementValue = sizeof(uint16_t);
    //16位为单位数字相加
    while (size > 1)
    {
        checkSum += *icmp++;     // 对传入的数据以uint16_t方式解析
        size -= decrementValue;
    }
    //长度奇数情况 size == 1
    if (size)
    {
        checkSum += *(uint8_t *) icmp;
    }
    //高位有进位，进位到低位，下面两行代码保证了高16位为0。
    while (checkSum >> 16)
    {
        checkSum = (checkSum >> 16) + (checkSum & 0xffff);
    }
    return (checkSum == 0xffff);
}

/**
calculateICMPSize
*/
int calculateICMPSize(void *icmp)
{
    int offset = sizeof(uint64_t);
    uint8_t *data = ((uint8_t *) icmp) + offset;
    return offset + (int) strlen((const char *) data) + 1;
}
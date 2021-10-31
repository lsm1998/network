//
// Created by Administrator on 2021/10/31.
//

#ifndef NETWORK_LOGIC_H
#define NETWORK_LOGIC_H

#include "ngx_c_socket.h"

class CLogicSocket : public CSocekt
{
public:
    // 构造函数
    CLogicSocket();
    // 释放函数
    virtual ~CLogicSocket();
    // 初始化函数
    virtual bool Initialize();
};

#endif //NETWORK_LOGIC_H

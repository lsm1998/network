//
// Created by Administrator on 2021/10/31.
//

#ifndef NETWORK_C_SOCKET_H
#define NETWORK_C_SOCKET_H

#include <list>
#include <csignal>
#include <global.h>
#include <config.h>
#include <sys/ioctl.h>
#include <map>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <log.h>

class CSocekt
{
private:
    // epoll连接的最大项数
    int m_worker_connections;
    // 所监听的端口数量
    int m_ListenPortCount;
    // 等待这么些秒后才回收连接
    int m_RecyConnectionWaitTime;

protected:
    int m_iWaitTime;                           //多少秒检测一次是否 心跳超时，只有当Sock_WaitTimeEnable = 1时，本项才有用

    struct ThreadItem
    {
        //线程句柄
        pthread_t _handle{};
        //记录线程池的指针
        CSocekt *p_this;
        //标记是否正式启动起来，启动起来后，才允许调用StopAll()来释放
        bool running;

        //构造函数
        ThreadItem(CSocekt *p_this) : p_this(p_this), running(false)
        {}

        //析构函数
        ~ThreadItem() = default;
    };

public:
    // 构造函数
    CSocekt() = default;

    // 释放函数
    virtual ~CSocekt();

    // 初始化函数[父进程中执行]
    virtual bool Initialize();

    bool ngx_open_listening_sockets();

    void init_conf();

    bool setnonblocking(int sock_fd);

    void ngx_close_listening_sockets();
};

#endif //NETWORK_C_SOCKET_H

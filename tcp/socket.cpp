//
// Created by Administrator on 2021/9/5.
//

#include "socket.h"

mySocket::MySocket::MySocket(const std::string &adder, unsigned short port)
{
    this->port = port;
    this->adder = adder;
}

int mySocket::MySocket::Close()
{
    return closesocket(fd);
}

int mySocket::MySocket::Connect()
{
    int ret;
    // 1创建fd
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd < 0)
    {
        return this->fd;
    }
    sockaddr_in sockAddr = {};
    // 2设置地址信息
    sockAddr.sin_port = htons(this->port);
    sockAddr.sin_family = AF_INET;
    inet_pton(AF_INET, this->adder.c_str(), &sockAddr.sin_addr);
    // 3建立连接
    ret = connect(this->fd, (struct sockaddr *) &sockAddr, sizeof(sockAddr));

//    std::call_once(this->oc, [&]()
//    {
//        try
//        {
//
//        }catch (...)
//        {
//
//        }
//    });
    return ret;
}

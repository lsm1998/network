//
// Created by Administrator on 2021/9/5.
//

#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#ifdef WIN32
#include <windows.h>
#elif __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <unistd.h>

#define closesocket close
#endif

#include <string>
#include <atomic>


namespace mySocket
{
    class MySocket
    {
    private:
        std::string adder;
        unsigned short port;
        int fd;

    public:
        std::once_flag oc;


    public:
        MySocket() : MySocket(0)
        {}

        MySocket(unsigned short port) : MySocket("", port)
        {}

        MySocket(const std::string &adder, unsigned short port);

        int Connect();

        int Close();
    };
}


#endif //NETWORK_SOCKET_H

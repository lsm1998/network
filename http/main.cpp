//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>
#include <regex>
#include <sys/stat.h>
#include "http_common.h"

#ifdef WIN32

#endif

class ThreadHandler
{
public:
    XTcp conn;

    ThreadHandler()
    {
        std::cout << "一个连接加入" << std::endl;
    }

    ~ThreadHandler()
    {
        std::cout << "一个连接退出" << std::endl;
    }

    void Main()
    {
        while (true)
        {
            HTTPRequest request(conn.sockFd);
            if (request.isBadRequest())
            {
                std::cout << "Bad Request" << std::endl;
                break;
            }
            std::cout << request.getType() << std::endl;
            std::cout << request.getPath() << std::endl;

            HTTPResponse response;
            if (request.getType() == "GET")
            {
                std::string filename = request.getPath();
                if (filename.find_first_of('/') == 0)
                {
                    filename = filename.substr(1, filename.length());
                }
                struct stat fileStat{};
                int ret = stat(filename.c_str(), &fileStat);
                if (ret < 0 || S_ISDIR(fileStat.st_mode))
                {
                    response.doNotFind(conn.sockFd);
                    break;
                }
                sendFile(conn.sockFd, fileStat.st_size, filename);
                break;
            } else
            {
                response.doNotFind(conn.sockFd);
            }
        }
        delete this;
    }
};

void bioServer()
{
    auto *server = new XTcp(8888);
    server->CreateSocket();
    server->Bind();
    // server->SetBlock(true);
    for (int i = 0; i < 1000; ++i)
    {
        auto conn = server->Accept();
        if (conn.sockFd <= 0)
        {
            break;
        }
        auto *t = new ThreadHandler();
        t->conn = conn;
        std::thread th(&ThreadHandler::Main, t);
        th.detach();
    }
}

void epollServer();

void selectServer();

int main()
{
    std::cout << "work dir=" << getcwd(nullptr, 0) << std::endl;
    bioServer();

    // selectServer();
}
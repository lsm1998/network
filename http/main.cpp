//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>
#include <regex>
#include "http_request.h"
#include "http_response.h"

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
        http_request request(conn.sockFd);
        http_response response(conn.sockFd);

        if (request.is_bad_request())
        {
            std::cout << "Bad Request" << std::endl;
        } else
        {
            if (response.send_static(request.get_path()) == -1)
            {
                std::string name = request.query("name");
                response.write_json(200, "{\"name\":123}");
            }
        }
        conn.Close();
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
//    auto *server = new XTcp();
//    if (server->Connect("127.0.0.1", 8888) < 0)
//    {
//        perror("Connect fail");
//        return 0;
//    }
//    for (int i = 0; i < 10; ++i)
//    {
//        printf("%d \n",server->Send("hello", 5));
//    }
    std::cout << "work dir=" << getcwd(nullptr, 0) << std::endl;
    bioServer();
}
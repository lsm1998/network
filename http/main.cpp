//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>
#include <cstring>

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
        char buf[1024] = {0};
        while (true)
        {
            int len = conn.Receive(buf, sizeof(buf) - 1);
            if (len == 0 || strcmp(buf, "exit") == 0)
            {
                break;
            } else if (len == -1)
            {
                perror("Receive fail");
                break;
            }
            std::cout << "收到数据:" << buf << std::endl;
            conn.Send(buf, len);
        }
        delete this;
    }
};

int main()
{
    auto *server = new XTcp(8080);
    server->CreateSocket();
    server->Bind();
    for (int i = 0; i < 1000; ++i)
    {
        auto conn = server->Accept();
        auto *t = new ThreadHandler();
        t->conn = conn;
        std::thread th(&ThreadHandler::Main, t);
        th.detach();
    }
}
//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>

class ThreadHandler
{
private:
    XTcp conn;

public:
    ThreadHandler(XTcp conn)
    {
        this->conn = conn;
    }

    void Main()
    {
        char buf[1024];
        int len = conn.Receive(buf, sizeof(buf));
        conn.Send(buf, len);
    }
};

int main()
{
    auto *server = new XTcp();
    server->CreateSocket();
    server->Bind();
    while (true)
    {
        auto client = server->Accept();
        auto *t = new ThreadHandler(client);
        std::thread th(&ThreadHandler::Main,t);
    }
}
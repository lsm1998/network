//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>

int main()
{
    auto *server = new XTcp();
    server->CreateSocket();
    server->Bind();
    while (true)
    {
        auto client = server->Accept();
        char buf[1024];
        int len = client.Receive(buf, sizeof(buf));
        if (len <= 0)
        {
            break;
        }
        client.Send(buf, len);
    }
}
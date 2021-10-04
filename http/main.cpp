//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>
#include <regex>

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
        char buf[1024] = {0};
        while (true)
        {
            //接受http客户端请求
            int len = conn.Receive(buf, sizeof(buf) - 1);
            if (len <= 0)
            {
                break;
            }
            buf[len] = '\0';
            printf("=======Receive=========\n%s===================\n", buf);
            //GET /index.html HTTP/1.1
            //Host: 192.168.3.69
            //User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:51.0) Gecko/20100101 Fi
            //Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
            //Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3
            //Accept-Encoding: gzip, deflate
            //DNT: 1
            //Connection: keep-alive
            //Upgrade-Insecure-Requests: 1
            std::string src = buf;
            std::string pattern = "^([A-Z]+) (.+) HTTP/1";
            std::regex r(pattern);
            std::smatch mas;
            regex_search(src, mas, r);
            if (mas.empty())
            {
                printf("%s failed!\n", pattern.c_str());
                break;
            }
            std::string type = mas[1];
            std::string path = mas[2];
            if (type != "GET")
            {
                break;
            }
            std::string filename = path;
            if (path == "/")
            {
                filename = "index.html";
            }
            std::string filepath = filename;
            FILE *fp = fopen(filepath.c_str(), "rb");
            if (fp == nullptr)
            {
                break;
            }
            //获取文件大小
            fseek(fp, 0, SEEK_END);
            int filesize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            printf("file size is %d\n", filesize);

            //回应http GET请求
            //消息头
            std::string rmsg;
            rmsg = "HTTP/1.1 200 OK\r\n";
            rmsg += "Server: XHttp\r\n";
            rmsg += "Content-Type: text/html\r\n";
            rmsg += "Content-Length: ";
            char bsize[128] = {0};
            sprintf(bsize, "%d", filesize);
            rmsg += bsize;

            rmsg += "\r\n\r\n";
            //发送消息头
            int sendSize = conn.Send(rmsg.c_str(), rmsg.size());
            printf("sendsize = %d\n", sendSize);
            printf("=======send=========\n%s\n=============\n", rmsg.c_str());
            //发送正文
            for (;;)
            {
                int len = fread(buf, 1, sizeof(buf), fp);
                if (len <= 0)break;
                int re = conn.Send(buf, len);
                if (re <= 0)break;
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
    // bioServer();

    selectServer();
}
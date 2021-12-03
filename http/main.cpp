//
// Created by Administrator on 2021/9/6.
//
#include <xtcp.h>
#include <thread>
#include <regex>
#include "http_request.h"
#include "http_response.h"
#include "sha1.h"
#include "base64.h"

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
    std::cout << "work dir=" << getcwd(nullptr, 0) << std::endl;
//    bioServer();
//    std::string str = "POST /www.baidu.com HTTP/1.1\r\n"
//                      "Host: miao.baidu.com\r\n"
//                      "Connection: keep-alive\r\n"
//                      "Content-Length: 4236\r\n"
//                      "sec-ch-ua: \"Chromium\";v=\"94\", \"Google Chrome\";v=\"94\", \";Not A Brand\";v=\"99\"\r\n"
//                      "sec-ch-ua-mobile: ?0\r\n"
//                      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/94.0.4606.61 Safari/537.36\n"
//                      "sec-ch-ua-platform: \"Windows\"\r\n"
//                      "Content-Type: text/plain;charset=UTF-8\r\n"
//                      "Accept: */*\r\n"
//                      "Origin: https://fanyi.baidu.com\r\n"
//                      "Sec-Fetch-Site: same-site\r\n"
//                      "Sec-Fetch-Mode: cors\r\n"
//                      "Sec-Fetch-Dest: empty\r\n"
//                      "Referer: https://fanyi.baidu.com/\r\n"
//                      "Accept-Encoding: gzip, deflate, br\r\n"
//                      "Accept-Language: zh-CN,zh;q=0.9\r\n"
//                      "Cookie: BIDUPSID=B0321F29303010EDF01F90A50E594B2F; PSTM=1614168850; ab_jid=75893c4a6b9dfad7102cb55bee284fb092f6; ab_jid_BFESS=75893c4a6b9dfad7102cb55bee284fb092f6; BAIDUID=B0321F29303010ED1BE5B546FE2FF16A:SL=0:NR=10:FG=1; __yjs_duid=1_7b117798c0569899c8aba2b0c493f25e1619967205454; BDUSS=kE2an5xNnRKQXZVZWg4dkY5an4taThCVUg3MERufldQampYZGNrWVBySUZ2WEpoSUFBQUFBJCQAAAAAAAAAAAEAAAAcjNST17-2-7K7t7JEaXNzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUwS2EFMEthN; BDUSS_BFESS=kE2an5xNnRKQXZVZWg4dkY5an4taThCVUg3MERufldQampYZGNrWVBySUZ2WEpoSUFBQUFBJCQAAAAAAAAAAAEAAAAcjNST17-2-7K7t7JEaXNzAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUwS2EFMEthN; BAIDUID_BFESS=B0321F29303010ED1BE5B546FE2FF16A:SL=0:NR=10:FG=1; BDORZ=B490B5EBF6F3CD402E515D22BCDA1598; ab_bid=56389d1225d8e7083a9773acc951f88ac4eb; ab_sr=1.0.1_MGI1ZGMxYjU4OGRlMjRjNGJkZTBlOGY2MjcwYThlYTdhNDlkNzZlMmJjYzM0OTUzNTk2ZDJlNDg1MzI3MDg5MzU5ZGRiYTdmM2ZkMWIxOWFlOTY0OTNkOGE1MTNmNjIyN2Y0M2QzODdiYjUzOTBlNzRkM2JiY2E4YmY1MTRhZTU5MmQxNTQyMzY3ZmQyNDVhYTliYmRjYTY3NTc4ZjgxZQ==; H_PS_PSSID=34646_34447_34527_34067_34749_34551_34742_34525_34584_34505_26350_34627_34701_34675; delPer=0; PSINO=6; BA_HECTOR=05810g8085a1ak8h0e1gltiqe0q\r\n\r\n123456";
//    HTTPRequest request(str);
//    std::cout << request.getHead("Content-Length");
}
//
// Created by Administrator on 2021/10/18.
//

#include "websocket.h"

websocket_server::~websocket_server()
{

}

void websocket_server::run()
{
    auto *server = new XTcp(8888);
    server->CreateSocket();
    server->Bind();

    for (int i = 0; i < 1000; ++i)
    {
        auto conn = server->Accept();
        if (conn.sockFd <= 0)
        {
            break;
        }
        std::thread th([=](int) -> void
                       {
                           int result = upgrade(conn.sockFd);
                           if (result < 0)
                           {
                               conn.Close();
                               return;
                           }
                           while (true)
                           {
                               if (this->read(conn.sockFd) < 0)
                               {
                                   break;
                               }
                           }
                       }, conn.sockFd);
        th.detach();
    }
}

int websocket_server::upgrade(int fd)
{
    http_request request(fd);
    http_response response(fd);

    if (request.is_bad_request())
    {
        return -1;
    }

    if (request.get_path() != this->path)
    {
        return -1;
    }
    if (request.get_header(upgrade_key) != upgrade_value)
    {
        return -1;
    }
    if (request.get_header(connection_key) != connection_value)
    {
        return -1;
    }
    if (request.get_header(sec_websocket_version_key) != sec_websocket_version_value)
    {
        return -1;
    }
    String accept = this->parse_sec_websocket_key(sec_websocket_key_key);
    response.set_code(101);
    response.set_head(connection_key, connection_value);
    response.set_head(upgrade_key, upgrade_value);
    response.set_head(sec_websocket_accept_key, accept);
    return response.send();
}

websocket_server::String websocket_server::parse_sec_websocket_key(const String &sec_websocket_key_key)
{
    common::SHA1 sha1 = {};
    sha1.Reset();
    unsigned int message_digest[5];
    sha1 << sec_websocket_key_key.c_str();
    sha1.Result(message_digest);
    for (unsigned int &i: message_digest)
    {
        i = htonl(i);
    }
    return common::base64_encode(reinterpret_cast<const unsigned char *>(message_digest), sizeof(message_digest));
}

int websocket_server::read(int fd)
{
    auto *f = new frame(fd);

}

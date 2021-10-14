//
// Created by Administrator on 2021/10/14.
//

#ifndef NETWORK_HTTP_RESPONSE_H
#define NETWORK_HTTP_RESPONSE_H

#include <string>
#include <map>
#include <sys/stat.h>
#include <cstring>

class http_response
{
    using String = std::string;

    using response_header = std::map<String, String>;

private:
    int sock_fd;

    int code;

    char *body;

    response_header header{};

    String root_dir;

private:
    int send();

public:
    http_response(int fd) : http_response("", fd)
    {}

    http_response(String root_dir, int fd);

    ~http_response();

    void write_json(int code, const String &json_str);

    int send_static(String filename);
};


#endif //NETWORK_HTTP_RESPONSE_H

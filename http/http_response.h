//
// Created by Administrator on 2021/10/14.
//

#ifndef NETWORK_HTTP_RESPONSE_H
#define NETWORK_HTTP_RESPONSE_H

#include <string>
#include <map>

class http_response
{
    using String = std::string;

    using response_header = std::map<String, String>;

private:
    int sock_fd;

    int code;

    char *body;

    response_header header{};

public:
    http_response(int fd);

    void write_json(int code, String json_str);
};


#endif //NETWORK_HTTP_RESPONSE_H

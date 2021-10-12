//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#include <string>
#include <map>
#include <sys/socket.h>
#include <unistd.h>

class http_request
{
    using String = std::string;

    using http_head = std::map<String, String>;

private:
    bool bad_request;

    String type;

    String path;

    char *body;

    http_head head{};

public:
    explicit http_request(int fd);

    explicit http_request(const char *content);

    explicit http_request(const String &content) : http_request(content.c_str())
    {};

    [[nodiscard]] String getType() const;

    [[nodiscard]] String getPath() const;

    [[nodiscard]] bool isBadRequest() const;

    [[nodiscard]] String getBody() const;

    [[nodiscard]] http_head getHead() const;

    String getHead(const String &key) const;
};

#endif //NETWORK_HTTP_COMMON_H

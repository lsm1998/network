//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <regex>

constexpr char *responseLine = (char *) "HTTP/1.1 200 OK\r\n";

class HTTPRequest
{
private:
    bool badRequest;
    std::string type;
    std::string path;

public:
    HTTPRequest(int fd);

    std::string getType() const;

    std::string getPath() const;

    bool isBadRequest() const;
};

class HTTPResponse
{
private:
    std::string responseBody;

    size_t bodySize;

public:
    HTTPResponse() = default;

    HTTPResponse *ok();

    HTTPResponse *setContentType(const std::string &type);
    // Content-Type: application/json

    // Content-Length

    HTTPResponse *setBody(char *body);

    HTTPResponse* setServer(const std::string& server);

    size_t doSend(int fd);
};

extern void sendHTML(int fd, const std::string &body);

#endif //NETWORK_HTTP_COMMON_H

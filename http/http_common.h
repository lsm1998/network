//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#ifdef __linux__
#include <sys/sendfile.h>
#endif
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <regex>
#include <sys/fcntl.h>
#include <map>

class HTTPRequest
{
    using String = std::string;

    using HttpHead = std::map<String, String>;

private:
    bool badRequest;

    String type;

    String path;

    char* body;

    HttpHead head{};

public:
    explicit HTTPRequest(int fd);

    explicit HTTPRequest(const char *content);

    explicit HTTPRequest(const String &content) : HTTPRequest(content.c_str())
    {};

    [[nodiscard]] String getType() const;

    [[nodiscard]] String getPath() const;

    [[nodiscard]] bool isBadRequest() const;

    [[nodiscard]] String getBody() const;

    [[nodiscard]] HttpHead getHead() const;

    String getHead(const String& key) const;
};

class HTTPResponse
{
private:
    int code{};

    std::string contentType;

    std::string server;

    char *body{};

    size_t contentLength{};

    bool close;

public:
    HTTPResponse() = default;

    HTTPResponse *ok();

    HTTPResponse *setCode(int code);

    HTTPResponse *setContentType(const std::string &type);

    HTTPResponse *setContentLength(const size_t &contentLength);

    HTTPResponse *setBody(char *body);

    HTTPResponse *setServer(const std::string &server);

    HTTPResponse *setConnectionClose();

    size_t doSend(int fd);

    void doNotFind(int fd);

    void doError(int fd, const std::string &error);
};

extern size_t sendHTML(int fd, const std::string &body);

extern size_t sendJSON(int fd, const std::string &body);

extern size_t sendFile(int fd, size_t size, const std::string &filename);

extern std::vector<std::string> split(const std::string& str, const std::string& regex);

#endif //NETWORK_HTTP_COMMON_H

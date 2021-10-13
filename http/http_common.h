//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#include <unistd.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cerrno>

extern ssize_t recv_peek(int sock_fd, void *buf, ssize_t len);

extern ssize_t readn(int fd, void *buf, ssize_t count);

extern ssize_t readLine(int sock_fd, void *buf, ssize_t max_line);

//class HTTPResponse
//{
//private:
//    int code{};
//
//    std::string contentType;
//
//    std::string server;
//
//    char *body{};
//
//    size_t contentLength{};
//
//    bool close;
//
//public:
//    HTTPResponse() = default;
//
//    HTTPResponse *ok();
//
//    HTTPResponse *setCode(int code);
//
//    HTTPResponse *setContentType(const std::string &type);
//
//    HTTPResponse *setContentLength(const size_t &contentLength);
//
//    HTTPResponse *setBody(char *body);
//
//    HTTPResponse *setServer(const std::string &server);
//
//    HTTPResponse *setConnectionClose();
//
//    size_t doSend(int fd);
//
//    void doNotFind(int fd);
//
//    void doError(int fd, const std::string &error);
//};
//
//extern size_t sendHTML(int fd, const std::string &body);
//
//extern size_t sendJSON(int fd, const std::string &body);
//
//extern size_t sendFile(int fd, size_t size, const std::string &filename);
//
//extern std::vector<std::string> split(const std::string& str, const std::string& regex);

#endif //NETWORK_HTTP_COMMON_H

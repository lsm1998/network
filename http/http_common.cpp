//
// Created by 刘时明 on 2021/10/4.
//

#include "http_common.h"

HTTPRequest::HTTPRequest(int fd)
{
    char buf[1024];
    size_t len = recv(fd, buf, sizeof(buf) - 1, 0);
    if (len <= 0)
    {
        this->badRequest = true;
        return;
    }
    buf[len] = '\0';

    std::string src = buf;
    std::string pattern = "^([A-Z]+) (.+) HTTP/1";
    std::regex r(pattern);
    std::smatch mas;
    regex_search(src, mas, r);
    if (mas.empty())
    {
        this->badRequest = true;
        return;
    }
    this->type = mas[1];
    this->path = mas[2];
}

std::string HTTPRequest::getType() const
{
    return this->type;
}

std::string HTTPRequest::getPath() const
{
    return this->path;
}

bool HTTPRequest::isBadRequest() const
{
    return this->badRequest;
}

HTTPResponse *HTTPResponse::ok()
{
    this->responseBody += responseLine;
    return this;
}

HTTPResponse *HTTPResponse::setContentType(const std::string &type)
{
    this->responseBody += "Content-Type: " + type + "\r\n";
    return this;
}

size_t HTTPResponse::doSend(int fd)
{
    return send(fd, this->responseBody.c_str(), this->responseBody.length(), 0);
}

HTTPResponse *HTTPResponse::setBody(char *body)
{
    this->bodySize = strlen(body);
    this->responseBody += "Content-Length: " + std::to_string(this->bodySize) + "\r\n\r\n";
    this->responseBody += body;
    return this;
}

HTTPResponse *HTTPResponse::setServer(const std::string &server)
{
    this->responseBody += "Server: " + server + "\r\n";
    return this;
}

void sendHTML(int fd, const std::string &body)
{
    HTTPResponse response = {};
    size_t result = response.ok()->
            setServer("Tengine")->
            setContentType("text/html")->
            setBody(const_cast<char *>(body.c_str()))->
            doSend(fd);
    printf("回复结果=%zu", result);
    printf("回复内容=%s", body.c_str());
}

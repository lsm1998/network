//
// Created by 刘时明 on 2021/10/4.
//

#include "http_common.h"

HTTPRequest::HTTPRequest(int fd)
{
    std::string reqContent;
    while (true)
    {
        char buf[1024];
        size_t len = recv(fd, buf, sizeof(buf) - 1, 0);
        if (len <= 0)
        {
            break;
        }
        reqContent += buf;
    }
    if (reqContent.empty())
    {
        this->badRequest = true;
        return;
    }
    String pattern = "^([A-Z]+) (.+) HTTP/1";
    std::regex r(pattern);
    std::smatch mas;
    regex_search(reqContent, mas, r);
    if (mas.empty())
    {
        this->badRequest = true;
        return;
    }
    this->type = mas[1];
    this->path = mas[2];

    // 获取请求头部分

    this->body = reqContent;

    printf("buf=%s \n", reqContent.c_str());
}

HTTPRequest::String HTTPRequest::getType() const
{
    return this->type;
}

HTTPRequest::String HTTPRequest::getPath() const
{
    return this->path;
}

bool HTTPRequest::isBadRequest() const
{
    return this->badRequest;
}

HTTPRequest::String HTTPRequest::getBody() const
{
    return this->body;
}

HTTPRequest::HttpHead HTTPRequest::getHttpHead() const
{
    return this->head;
}

HTTPResponse *HTTPResponse::ok()
{
    this->code = 200;
    return this;
}

HTTPResponse *HTTPResponse::setContentType(const std::string &type)
{
    this->contentType = type;
    return this;
}

size_t HTTPResponse::doSend(int fd)
{
    // 组装数据
    std::string responseBuff;
    responseBuff += "HTTP/1.1 " + std::to_string(this->code) + " OK\r\n";
    responseBuff += this->close ? "Connection: Close\r\n" : "";
    responseBuff += "Server: " + this->server + "\r\n";
    responseBuff += "Content-Type: " + this->contentType + "\r\n";
    responseBuff += "Content-Length: " + std::to_string(this->contentLength) + "\r\n\r\n";
    responseBuff += this->body == nullptr ? "" : this->body;
    return send(fd, responseBuff.c_str(), responseBuff.length(), 0);
}

HTTPResponse *HTTPResponse::setBody(char *body)
{
    this->body = body;
    this->contentLength = strlen(body);
    return this;
}

HTTPResponse *HTTPResponse::setServer(const std::string &server)
{
    this->server += server;
    return this;
}

void HTTPResponse::doNotFind(int fd)
{
    std::string body = "<html><title>Not Find Error</title>"
                       "<body>"
                       "<h1>404</h1>"
                       "<p>GET: Can't find the file</p>"
                       "</body></html>";
    sendHTML(fd, body);
}

void HTTPResponse::doError(int fd, const std::string &error)
{
    sendHTML(fd, "<html><title>Not Find Error</title>"
                 "<body>"
                 "<h1>404</h1>"
                 "<p>" + error + "</p>"
                                 "</body></html>");
}

HTTPResponse *HTTPResponse::setCode(int code)
{
    this->code = code;
    return this;
}

HTTPResponse *HTTPResponse::setContentLength(const size_t &contentLength)
{
    this->contentLength = contentLength;
    return this;
}

HTTPResponse *HTTPResponse::setConnectionClose()
{
    this->close = true;
    return this;
}

size_t sendHTML(int fd, const std::string &body)
{
    HTTPResponse response = {};
    return response.ok()->
            setServer("lsm-server")->
            setContentType("text/html")->
            setBody(const_cast<char *>(body.c_str()))->
            doSend(fd);
}

size_t sendJSON(int fd, const std::string &body)
{
    HTTPResponse response = {};
    return response.ok()->
            setServer("lsm-server")->
            setContentType("application/json")->
            setBody(const_cast<char *>(body.c_str()))->
            doSend(fd);
}

size_t sendFile(int fd, size_t size, const std::string &filename)
{
    HTTPResponse response;
    response.ok()->
            setContentLength(size)->
            doSend(fd);
    int file_fd = open(filename.c_str(), O_RDONLY);
#ifdef __APPLE__
    off_t len{};
    sendfile(file_fd, fd, 0, &len, nullptr, 0);
#endif
    return 0;
}

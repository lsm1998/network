//
// Created by 刘时明 on 2021/10/4.
//

#include "http_common.h"

HTTPRequest::HTTPRequest(int fd)
{
    char buf[BUFF_SIZE];
    size_t len = recv(fd, buf, sizeof(buf) - 1, 0);
    if (len <= 0)
    {
        this->badRequest = true;
        return;
    }
    buf[len] = '\0';
    new(this) HTTPRequest(buf);
}

HTTPRequest::HTTPRequest(const char *content)
{
    std::string reqContent = content;
    if (reqContent.empty())
    {
        this->badRequest = true;
        return;
    }

    // 是否有请求体
    u_long index = reqContent.find("\r\n\r\n");
    if (index == -1 || index > reqContent.length())
    {
        this->badRequest = true;
        return;
    } else if (index < reqContent.length() - 4)
    {
        // 有请求体
        this->body = reqContent.substr(index + 4, reqContent.length()).data();
    }
    reqContent = reqContent.substr(0, index);
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
    auto list = split(reqContent, "\r\n");
    for (int i = 1; i < list.size(); ++i)
    {
        String &temp = list[i];
        index = temp.find_first_of(": ");
        if (index == 0)
        {
            continue;
        }
        this->head[temp.substr(0, index)] = temp.substr(index + 2, temp.length());
    }
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

HTTPRequest::HttpHead HTTPRequest::getHead() const
{
    return this->head;
}

HTTPRequest::String HTTPRequest::getHead(const HTTPRequest::String &key) const
{
    auto val = this->head.find(key);
    return val->second;
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
    // responseBuff += this->close ? "Connection: Close\r\n" : "";
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
    off_t len{};
#ifdef __APPLE__
    return sendfile(file_fd, fd, 0, &len, nullptr, 0);
#elif __linux__
    return sendfile(fd, file_fd, &len, size);
#endif
    return 0;
}

std::vector<std::string> split(const std::string &str, const std::string &regex)
{
    std::regex re(regex);
    std::sregex_token_iterator first{str.begin(), str.end(), re, -1}, last;
    return {first, last};
}

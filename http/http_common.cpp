//
// Created by 刘时明 on 2021/10/4.
//

#include "http_common.h"

HTTPRequest::HTTPRequest(int fd)
{
    char buf[1024];
    int len = read(fd, buf, sizeof(buf) - 1);
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

std::string HTTPRequest::getType()
{
    return this->type;
}

std::string HTTPRequest::getPath()
{
    return this->path;
}

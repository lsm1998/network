//
// Created by Administrator on 2021/10/12.
//

#include "http_request.h"
#include "http_common.h"


http_request::http_request(int fd)
{
    // readLine(int sock_fd, void *buf, ssize_t max_line)
    while (true)
    {
        char buf[MAX_BUF_SIZE];
        ssize_t t = readLine(fd, buf, MAX_BUF_SIZE);
    }

}

http_request::String http_request::getType() const
{
    return http_request::String();
}

http_request::String http_request::getPath() const
{
    return http_request::String();
}

bool http_request::isBadRequest() const
{
    return false;
}

char *http_request::getBody() const
{
    return this->body;
}

http_request::http_head http_request::getHead() const
{
    return http_request::http_head();
}

http_request::String http_request::getHead(const http_request::String &key) const
{
    return http_request::String();
}

ssize_t http_request::readLine()
{
    return 0;
}

//
// Created by Administrator on 2021/10/12.
//

#include "http_request.h"

http_request::http_request(int fd)
{

}

http_request::http_request(const char *content)
{

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

http_request::String http_request::getBody() const
{
    return http_request::String();
}

http_request::http_head http_request::getHead() const
{
    return http_request::http_head();
}

http_request::String http_request::getHead(const http_request::String &key) const
{
    return http_request::String();
}

ssize_t recv_peek(int sockfd, void *buf, ssize_t len)
{
    while (true)
    {
        ssize_t ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret == -1 && errno == EINTR)
            continue;
        return ret;
    }
}

ssize_t readn(int fd, void *buf, ssize_t count)
{
    ssize_t nleft = count;
    ssize_t nread;
    char *bufp = (char *) buf;

    while (nleft > 0)
    {
        if ((nread = read(fd, bufp, nleft)) < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        } else if (nread == 0)
        {
            return count - nleft;
        }
        bufp += nread;
        nleft -= nread;
    }
    return count;
}

ssize_t readLine(int sock_fd, void *buf, ssize_t max_line)
{
    ssize_t ret;
    ssize_t nread;
    char *bufp = static_cast<char *>(buf);
    ssize_t nleft = max_line;
    while (true)
    {
        ret = recv_peek(sock_fd, bufp, nleft);
        if (ret <= 0)
            return ret;
        nread = ret;
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')
            {
                ret = readn(sock_fd, bufp, i + 1);
                if (ret != i + 1)
                {
                    exit(EXIT_FAILURE);
                }
                return ret;
            }
        }
        if (nread > nleft)
            exit(EXIT_FAILURE);
        nleft -= nread;
        ret = readn(sock_fd, bufp, nread);
        if (ret != nread)
            exit(EXIT_FAILURE);
        bufp += nread;
    }
    return -1;
}
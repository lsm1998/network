//
// Created by 刘时明 on 2021/10/4.
//

#include "http_common.h"

ssize_t recv_peek(int sock_fd, void *buf, ssize_t len)
{
    while (true)
    {
        ssize_t ret = recv(sock_fd, buf, len, MSG_PEEK);
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

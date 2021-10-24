//
// Created by Administrator on 2021/10/12.
//

#include <http/http_request.h>

http_request::http_request(int fd)
{
    if (fd <= 0)
    {
        this->bad_request = true;
        perror("sock fd fail");
        return;
    }
    this->sock_fd = fd;
    std::vector<String> list;
    while (true)
    {
        char buf[MAX_BUF_SIZE];
        ssize_t len = read_line(buf, MAX_BUF_SIZE);
        if (len == MAX_BUF_SIZE)
        {
            this->bad_request = true;
            perror("line  fd fail");
            return;
        }
        buf[len] = '\0';
        String line = buf;
        // 请求头解析完毕
        if (line == "\r\n")
        {
            break;
        }
        if (line.find_last_of("\r\n"))
        {
            line = line.substr(0, line.length() - 2);
        }
        list.push_back(line);
        if (len == 0)
        {
            break;
        }
    }
    std::cout << list.size() << std::endl;
    // 解析请求行
    this->parse_line(list[0]);

    // 解析请求头
    this->parse_header(list);

    // 解析请求体
    this->parse_body();
}

http_request::String http_request::get_content_type() const
{
    return this->get_header("Content-Type");
}

http_request::String http_request::get_path() const
{
    return this->path;
}

bool http_request::is_bad_request() const
{
    return this->bad_request;
}

char *http_request::get_body() const
{
    return this->body;
}

http_request::http_header http_request::get_header() const
{
    return this->header;
}

http_request::String http_request::get_header(const http_request::String &key) const
{
    return this->header.count(key) ? this->header.find(key)->second : "";
}

ssize_t http_request::read_line(void *buf, ssize_t max_line)
{
    ssize_t ret;
    ssize_t nread;
    char *bufp = static_cast<char *>(buf);
    ssize_t nleft = max_line;
    while (true)
    {
        ret = this->recv_peek(bufp, nleft);
        if (ret <= 0)
            return ret;
        nread = ret;
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')
            {
                ret = this->readn(bufp, i + 1);
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
        ret = this->readn(bufp, nread);
        if (ret != nread)
            exit(EXIT_FAILURE);
        bufp += nread;
    }
    return -1;
}

ssize_t http_request::readn(void *buf, ssize_t count) const
{
    ssize_t nleft = count;
    ssize_t nread;
    char *bufp = (char *) buf;

    while (nleft > 0)
    {
        if ((nread = read(this->sock_fd, bufp, nleft)) < 0)
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

ssize_t http_request::recv_peek(void *buf, ssize_t len) const
{
    while (true)
    {
        ssize_t ret = recv(this->sock_fd, buf, len, MSG_PEEK);
        if (ret == -1 && errno == EINTR)
            continue;
        return ret;
    }
}

int http_request::parse_line(const String &line)
{
    std::vector<String> list = split(line, " ");
    if (list.size() != 3)
    {
        perror("http line format error");
        return -1;
    }
    this->method = std::move(list[0]);
    this->version = std::move(list[2]);

    auto index = list[1].find_first_of('?');
    if (index != -1)
    {
        this->path = list[1].substr(0, index);
        this->parse_query(&this->query_map, list[1].substr(index + 1, list[1].length()));
    } else
    {
        this->path = std::move(list[1]);
    }
    return 1;
}

int http_request::parse_header(const std::vector<String> &list)
{
    // remove_vector_first(list);
    for (auto &temp: list)
    {
        size_t index = temp.find_first_of(": ");
        if (index == -1)
        {
            continue;
        }
        this->header[temp.substr(0, index)] = temp.substr(index + 2, temp.length());
    }
    return 0;
}

http_request::String http_request::get_method() const
{
    return this->method;
}

http_request::String http_request::get_version() const
{
    return this->version;
}

int http_request::parse_body()
{
    String length_str = this->get_header("Content-Length");
    if (length_str.empty() || length_str == "0")
    {
        return 0;
    }
    int length = atoi(length_str.c_str());
    this->body = static_cast<char *>(malloc(length));
    ssize_t len = read(this->sock_fd, this->body, length);
    if (len != length)
    {
        return -1;
    }
    return 0;
}

http_request::~http_request()
{
    free(this->body);
}

int http_request::parse_query(form_map *map, const String &query_str)
{
    auto list = split(query_str, "&");
    for (auto &temp: list)
    {
        auto index = temp.find_first_of('=');
        String key;
        String value;
        if (index == -1)
        {
            key = temp;
            value = "";
        } else
        {
            key = temp.substr(0, index);
            value = temp.substr(index + 1, temp.length());
        }
        (*map)[key].push_back(value);
    }
    return 0;
}

http_request::String http_request::query(const http_request::String &key) const
{
    if (this->query_map.count(key))
    {
        return this->query_map.at(key)[0];
    } else
    {
        return "";
    }
}

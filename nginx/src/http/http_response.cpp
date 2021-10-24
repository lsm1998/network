//
// Created by Administrator on 2021/10/14.
//

#include <http/http_response.h>

http_response::http_response(http_response::String root_dir, int fd)
{
    this->sock_fd = fd;
    this->root_dir = std::move(root_dir);
}

http_response::~http_response()
{
    free(this->body);
}

void http_response::write_json(int code, const http_response::String &json_str)
{
    this->code = code;
    this->set_content_type("application/json; charset=utf-8");
    this->set_body(json_str.c_str(), json_str.size());
    this->send();
}

int http_response::send_static(http_response::String filename)
{
    filename = this->root_dir + filename;
    if (filename.find_first_of('/') == 0)
    {
        filename = filename.substr(1, filename.length());
    }
    struct stat fileStat{};
    int ret = stat(filename.c_str(), &fileStat);
    if (ret < 0 || S_ISDIR(fileStat.st_mode))
    {
        return -1;
    }
    int file_fd = open(filename.c_str(), O_RDONLY);
    off_t len{};
    this->set_code(200);
    this->set_content_length(fileStat.st_size);
    this->send();
#ifdef __APPLE__
    sendfile(file_fd, this->sock_fd, 0, &len, nullptr, 0);
#elif __linux__
    ssize_t result = sendfile(this->sock_fd, file_fd, &len, fileStat.st_size);
    return result > 0 ? 0 : -1;
#endif
    return -1;
}

int http_response::send()
{
    std::ostringstream buffer{};
    buffer << VERSION << BLANK;
    buffer << this->code << BLANK;
    buffer << CODE_MAP.at(this->code) << "\r\n";
    for (auto &v: this->header)
    {
        buffer << v.first << ": " << v.second << "\r\n";
    }
    buffer << "\r\n";
    String str = buffer.str();
    write(this->sock_fd, str.c_str(), str.length());
    write(this->sock_fd, this->body, this->content_length);
    return 0;
}

void http_response::set_body(const char *body, int length)
{
    this->body = static_cast<char *>(malloc(length));
    strcpy(this->body, body);
    this->header["Content-Length"] = std::to_string(length);
    this->content_length = length;
}

void http_response::set_content_type(const String &content_type)
{
    this->header["Content-Type"] = content_type;
}

void http_response::set_content_length(int length)
{
    this->header["Content-Length"] = std::to_string(length);
    this->content_length = length;
}

void http_response::set_code(int code)
{
    this->code = code;
}

void http_response::set_head(const http_response::String &key, const http_response::String &value)
{
    this->header[key] = value;
}



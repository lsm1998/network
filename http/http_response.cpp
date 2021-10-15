//
// Created by Administrator on 2021/10/14.
//

#include "http_response.h"

http_response::http_response(http_response::String root_dir, int fd)
{
    this->sock_fd = fd;
    this->root_dir = std::move(root_dir);
}

void http_response::write_json(int code, const http_response::String &json_str)
{
    this->code = code;
    this->header[""] = "";
    this->body = static_cast<char *>(malloc(json_str.length() + 1));
    strcpy(this->body, json_str.c_str());
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
#ifdef __APPLE__
    sendfile(file_fd, this->sock_fd, 0, &len, nullptr, 0);
#elif __linux__
    ssize_t result = sendfile(this->sock_fd, file_fd, &len, fileStat.st_size);
    return result > 0 ? 0 : -1;
#endif
    return -1;
}

http_response::~http_response()
{
    free(this->body);
}

int http_response::send()
{
    return 0;
}



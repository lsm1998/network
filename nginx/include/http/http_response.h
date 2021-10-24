//
// Created by Administrator on 2021/10/14.
//

#ifndef NETWORK_HTTP_RESPONSE_H
#define NETWORK_HTTP_RESPONSE_H

#include <unistd.h>
#include <string>
#include <map>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <cstring>
#include <sstream>
#include <fcntl.h>

// 101: "Protocols",
constexpr const char *VERSION = "HTTP/1.1";

constexpr const char *BLANK = " ";

const std::map<int, std::string> CODE_MAP = {
        {101, "Protocols"},
        {200, "OK"},
        {400, "Bad Request"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
};

class http_response
{
    using String = std::string;

    using response_header = std::map<String, String>;

private:
    int sock_fd;

    int code;

    char *body;

    response_header header{};

    String root_dir;

    int content_length;

public:
    http_response(int fd) : http_response("", fd)
    {}

    http_response(String root_dir, int fd);

    ~http_response();

    void write_json(int code, const String &json_str);

    int send_static(String filename);

    void set_body(const char *body, int length);

    void set_content_type(const String &content_type);

    void set_content_length(int length);

    void set_code(int code);

    void set_head(const String &key, const String &value);

    int send();
};


#endif //NETWORK_HTTP_RESPONSE_H

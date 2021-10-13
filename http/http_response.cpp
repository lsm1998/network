//
// Created by Administrator on 2021/10/14.
//

#include "http_response.h"

http_response::http_response(int fd)
{
    this->sock_fd = fd;
}

void http_response::write_json(int code, http_response::String json_str)
{
    this->code = code;
}

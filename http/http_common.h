//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#include <unistd.h>
#include <string>
#include <regex>

class HTTPRequest
{
private:
    bool badRequest;
    std::string type;
    std::string path;

public:
    HTTPRequest(int fd);

    std::string getType();

    std::string getPath();
};

#endif //NETWORK_HTTP_COMMON_H

//
// Created by Administrator on 2021/9/21.
//

#ifndef NETWORK_BASE64_H
#define NETWORK_BASE64_H

#include <string>
#include <iostream>

namespace common
{
    std::string base64_encode(unsigned char const *, unsigned int len);

    std::string base64_decode(std::string const &s);
}

#endif //NETWORK_BASE64_H

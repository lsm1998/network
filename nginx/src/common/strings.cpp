//
// Created by Administrator on 2021/10/26.
//

#include <strings.h>

std::vector<std::string> split(const std::string &str, const std::string &pattern)
{
    char *str_c = new char[strlen(str.c_str()) + 1];
    strcpy(str_c, str.c_str());
    std::vector<std::string> result;
    char *tmpStr = strtok(str_c, pattern.c_str());
    while (tmpStr != nullptr)
    {
        result.emplace_back(tmpStr);
        tmpStr = strtok(nullptr, pattern.c_str());
    }
    delete[] str_c;
    return result;
}

void trim(std::string &str)
{
    if (!str.empty())
    {
        str.erase(0, str.find_first_not_of(' '));
        str.erase(str.find_last_not_of(' ') + 1);
    }
}
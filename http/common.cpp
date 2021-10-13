//
// Created by Administrator on 2021/10/13.
//

#include "common.h"

std::vector<std::string> split(const std::string &str, const std::string &pattern)
{
    //const char* convert to char*
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

template<typename T>
int remove_vector_first(const std::vector<T> &list)
{
    if (list.empty())
    {
        return -1;
    }
    auto first = list.begin();
    list.erase(first);
    return 0;
}
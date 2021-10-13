//
// Created by Administrator on 2021/10/13.
//

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <string>
#include <cstring>
#include <vector>

std::vector <std::string> split(const std::string &str, const std::string &pattern);

template<typename T>
int remove_vector_first(const std::vector<T> &list);

#endif //NETWORK_COMMON_H

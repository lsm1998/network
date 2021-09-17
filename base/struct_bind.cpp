//
// Created by Administrator on 2021/9/17.
//
#include <map>
#include <iostream>

void structBindTest()
{
    std::map<std::string, int> m;
    const auto[key, value] =m.insert(std::pair<std::string, int>("hello", 100));
    std::cout << key->first << std::endl;
    std::cout << key->second << std::endl;
    std::cout << "insert is success? " << value << std::endl;
    m["age"] = 23;

    for (auto &[k, v]: m)
    {
        std::cout << k << std::endl;
        std::cout << v << std::endl;
    }
}
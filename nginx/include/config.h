//
// Created by Administrator on 2021/10/26.
//

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <string>
#include <set>
#include <mutex>
#include <strings.h>
#include <fstream>
#include <thread>

class config_item
{
public:
    std::string item_key;
    std::string item_value;
};

using conf_item = config_item;

class nginx_config
{
private:
    static nginx_config *instance;

    static std::mutex m_mutex;

    static std::once_flag once_flag;

    std::set<conf_item *> item_set;

    nginx_config() = default;

    nginx_config(nginx_config &) = delete;

    nginx_config &operator=(const nginx_config &) = delete;

public:
    ~nginx_config();

    static nginx_config *getInstance();

    void load(const char *conf_name);

    const char *get_item(const std::string &key);

    const char *get_item_default(const std::string &key, const std::string &def_val);

    int get_item_int_default(const std::string &key, int def_val);

    class config_free //类中套类，用于释放对象
    {
    public:
        ~config_free()
        {
            if (nginx_config::instance)
            {
                delete nginx_config::instance;
                nginx_config::instance = nullptr;
            }
        }
    };
};

#endif //NETWORK_CONFIG_H

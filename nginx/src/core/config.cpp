//
// Created by Administrator on 2021/10/26.
//

#include <config.h>

nginx_config *nginx_config::instance = nullptr;

std::mutex m_mutex{};

std::once_flag nginx_config::once_flag{};

nginx_config::~nginx_config()
{
    std::set<conf_item *>::iterator pos;
    for (pos = this->item_set.begin(); pos != item_set.end(); ++pos)
    {
        delete (*pos);
    }
    item_set.clear();
}

nginx_config *nginx_config::getInstance()
{
    if (nginx_config::instance == nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (nginx_config::instance == nullptr)
        {
            nginx_config::instance = new nginx_config();
            static config_free cl;
        }
    }
    return nginx_config::instance;
}

const char *nginx_config::get_item(const std::string &key)
{
    std::set<conf_item *>::iterator pos;
    for (pos = this->item_set.begin(); pos != item_set.end(); ++pos)
    {
        if (key == (*pos)->item_key)
        {
            return (*pos)->item_value.c_str();
        }
    }
    return nullptr;
}

const char *nginx_config::get_item_default(const std::string &key, const std::string &def_val)
{
    const char *val = this->get_item(key);
    if (val == nullptr)
    {
        return def_val.c_str();
    }
    return val;
}

int nginx_config::get_item_int_default(const std::string &key, int def_val)
{
    const char *value = this->get_item(key);
    if (value == nullptr)
    {
        return def_val;
    }
    return atoi(value);
}

void nginx_config::load(const char *conf_name)
{
    std::call_once(once_flag, [](const char *conf_name, std::set<conf_item *> &set) -> void
    {
        std::ifstream input(conf_name);
        std::string line;
        if (input.is_open())
        {
            while (getline(input, line))
            {
                if (line.empty())
                {
                    continue;
                }
                trim(line);
                if (line.find_last_of('\r') == line.length() - 1)
                {
                    line = line.substr(0, line.length() - 1);
                }
                if (line == ";" ||
                    line == " " ||
                    line == "#" ||
                    line == "\t" ||
                    line == "\n" ||
                    line == "\r")
                    continue;
                // 是否注释行
                if (line.find('#') == 0 || line.find('[') == 0)
                {
                    continue;
                }
                size_t index = line.find('=');
                if (index > 0)
                {
                    auto *item = new conf_item();
                    item->item_key = line.substr(0, index);
                    item->item_value = line.substr(index + 1, line.length());
                    trim(item->item_key);
                    trim(item->item_value);
                    set.insert(item);
                }
            }
            printf("配置文件读取完毕！\n");
            input.close();
        }
    }, conf_name, std::ref(this->item_set));
}

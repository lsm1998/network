//
// Created by Administrator on 2021/10/24.
//

#include <banner.h>
#include <global.h>
#include <config.h>

int main(int argc, char *const *argv, char **env)
{
    // banner
    show_banner();

    // 进程ID信息
    nginx_pid = getpid();
    nginx_parent = getppid();

    for (int i = 0; i < argc; i++)
    {
        g_argv_need_mem += strlen(argv[i]) + 1;
    }

    // 统计环境变量所占的内存
    for (int i = 0; env[i]; i++)
    {
        g_env_need_mem += strlen(env[i]) + 1;
    }

    g_os_argc = argc;
    g_os_argv = (char **) argv;

    // 全局变量有必要初始化的
    nginx_process = PROCESS_MASTER;
    nginx_reap = 0;

    auto *config = nginx_config::getInstance();
    config->load(CONFIG_PATH);


    return 0;
}
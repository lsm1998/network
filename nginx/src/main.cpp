//
// Created by Administrator on 2021/10/24.
//

#include <log.h>
#include <banner.h>
#include <global.h>
#include <config.h>
#include <setproctitle.h>
#include <net.h>

// 保存argv参数所需要的内存大小
size_t g_argvneedmem = 0;
// 环境变量所占内存大小
size_t g_envneedmem = 0;
// 指向自己分配的env环境变量的内存
char *gp_envmem = nullptr;

char **g_os_env;

CLogicSocket g_socket;

int main(int argc, const char **argv, char **env)
{
    g_os_argc = argc;
    g_os_argv = (char **) argv;
    g_os_env = env;

    // show banner
    show_banner();

    // 进程ID信息
    nginx_pid = getpid();
    nginx_parent = getppid();

    g_argvneedmem = 0;
    for (int i = 0; i < argc; i++) // 统计argv所占的内存
    {
        // '\0' 需要一个字节
        g_argvneedmem += strlen(argv[i]) + 1;
    }

    for (int i = 0; g_os_env[i]; i++) // 统计环境变量所占的内存
    {
        // '\0' 需要一个字节
        g_envneedmem += strlen(g_os_env[i]) + 1;
    }

    // 日志初始化
    ngx_log_init();

    log_stderr(0, "start...");

    // 全局变量有必要初始化的
    nginx_process = PROCESS_MASTER;
    nginx_reap = 0;

    auto *config = nginx_config::getInstance();
    config->load(CONFIG_PATH);

    init_set_proc_title();

    set_proc_title("worker processes");

    g_socket.Initialize();
    return 0;
}
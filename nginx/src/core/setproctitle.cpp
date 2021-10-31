#include <setproctitle.h>

// 设置可执行程序标题相关函数：分配内存，并且把环境变量拷贝到新内存中来
void init_set_proc_title()
{
    gp_envmem = new char[g_envneedmem];
    memset(gp_envmem, 0, g_envneedmem);

    char *ptmp = gp_envmem;

    for (int i = 0; env[i]; i++)
    {
        // strlen不包括字符串末尾的'\0'
        size_t size = strlen(env[i]) + 1;
        // 原环境变量内容拷贝到新内存
        strcpy(ptmp, env[i]);
        // 新环境变量指向这段新内存
        env[i] = ptmp;
        ptmp += size;
    }
}

//设置可执行程序标题
void set_proc_title(const char *title)
{
    size_t title_len = strlen(title);

    // argv和environ内存总和
    size_t esy = g_argvneedmem + g_envneedmem;
    if (esy <= title_len)
    {
        // 标题太长
        return;
    }
    // 设置后续的命令行参数为空，表示只有argv[]中只有一个元素；防止后续argv被滥用，因为很多判断是用argv[] == NULL来做结束标记判断的;
    g_os_argv[1] = nullptr;

    // 把标题弄进来，注意原来的命令行参数都会被覆盖掉，不要再使用这些命令行参数,而且g_os_argv[1]已经被设置为NULL了
    char *ptmp = g_os_argv[0];
    strcpy(ptmp, title);
    // 跳过标题
    ptmp += title_len;

    // 把剩余的原argv以及environ所占的内存全部清0，否则会出现在ps的cmd列可能还会残余一些没有被覆盖的内容；
    size_t cha = esy - title_len;  // 内存总和减去标题字符串长度(不含字符串末尾的\0)，剩余的大小，就是要memset的；
    memset(ptmp, 0, cha);
}
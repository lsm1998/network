#include <net/ngx_c_socket.h>

//--------------------------------------------------------------------------
//构造函数
CSocekt::CSocekt()
{
    //配置相关
    m_worker_connections = 1;      //epoll连接最大项数
    m_ListenPortCount = 1;         //监听一个端口
    m_RecyConnectionWaitTime = 60; //等待这么些秒后才回收连接
}

//初始化函数【fork()子进程之前干这个事】
//成功返回true，失败返回false
bool CSocekt::Initialize()
{
    init_conf();  //读配置项
    // 打开监听端口
    return ngx_open_listening_sockets();
}

//--------------------------------------------------------------------------
//释放函数
CSocekt::~CSocekt()
{
    //释放必须的内存
    //(1)监听端口相关内存的释放--------
//    for (auto &pos: m_ListenSocketList)
//    {
//        // 一定要把指针指向的内存干掉，不然内存泄漏
//        delete pos;
//    }
//    m_ListenSocketList.clear();
}

//专门用于读各种配置项
void CSocekt::init_conf()
{
    auto *config = nginx_config::getInstance();
    // epoll连接的最大项数
    m_worker_connections = config->get_item_int_default("worker_connections", m_worker_connections);
    // 取得要监听的端口数量
    m_ListenPortCount = config->get_item_int_default("ListenPortCount", m_ListenPortCount);
    // 等待这么些秒后才回收连接
    m_RecyConnectionWaitTime = config->get_item_int_default("Sock_RecyConnectionWaitTime", m_RecyConnectionWaitTime);
    // 多少秒检测一次是否 心跳超时，只有当Sock_WaitTimeEnable = 1时，本项才有用
    m_iWaitTime = config->get_item_int_default("Sock_MaxWaitTime", m_iWaitTime);
    // 不建议低于5秒钟，因为无需太频繁
    m_iWaitTime = (m_iWaitTime > 5) ? m_iWaitTime : 5;
}

//监听端口【支持多个端口】，这里遵从nginx的函数命名
//在创建worker进程之前就要执行这个函数；
bool CSocekt::ngx_open_listening_sockets()
{
    int isock;                //socket
    struct sockaddr_in serv_addr{};            //服务器的地址结构体
    int iport;                //端口
    char strinfo[100];         //临时字符串

    //初始化相关
    memset(&serv_addr, 0, sizeof(serv_addr));  //先初始化一下
    serv_addr.sin_family = AF_INET;                //选择协议族为IPV4
    serv_addr.sin_addr.s_addr = htonl(
            INADDR_ANY); //监听本地所有的IP地址；INADDR_ANY表示的是一个服务器上所有的网卡（服务器可能不止一个网卡）多个本地ip地址都进行绑定端口号，进行侦听。
    //中途用到一些配置信息
    auto *config = nginx_config::getInstance();
    // 要监听这么多个端口
    for (int i = 0; i < m_ListenPortCount; i++)
    {
        //参数1：AF_INET：使用ipv4协议，一般就这么写
        //参数2：SOCK_STREAM：使用TCP，表示可靠连接【相对还有一个UDP套接字，表示不可靠连接】
        //参数3：给0，固定用法，就这么记
        isock = socket(AF_INET, SOCK_STREAM, 0); //系统函数，成功返回非负描述符，出错返回-1
        if (isock == -1)
        {
            //其实这里直接退出，那如果以往有成功创建的socket呢？就没得到释放吧，当然走到这里表示程序不正常，应该整个退出，也没必要释放了 
            return false;
        }

        //setsockopt（）:设置一些套接字参数选项；
        //参数2：是表示级别，和参数3配套使用，也就是说，参数3如果确定了，参数2就确定了;
        //参数3：允许重用本地地址
        //设置 SO_REUSEADDR，目的第五章第三节讲解的非常清楚：主要是解决TIME_WAIT这个状态导致bind()失败的问题
        int reuseaddr = 1;  //1:打开对应的设置项
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, (const void *) &reuseaddr, sizeof(reuseaddr)) == -1)
        {
            close(isock); //无需理会是否正常执行了
            return false;
        }

        //为处理惊群问题使用reuseport

        int reuseport = 1;
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEPORT, (const void *) &reuseport, sizeof(int)) == -1) //端口复用需要内核支持
        {
            //失败就失败吧，失败顶多是惊群，但程序依旧可以正常运行，所以仅仅提示一下即可
        }
        //设置该socket为非阻塞
        if (!setnonblocking(isock))
        {
            close(isock);
            return false;
        }
        //设置本服务器要监听的地址和端口，这样客户端才能连接到该地址和端口并发送数据        
        strinfo[0] = 0;
        sprintf(strinfo, "ListenPort%d", i);
        iport = config->get_item_int_default(strinfo, 10000);
        serv_addr.sin_port = htons((in_port_t) iport);   //in_port_t其实就是uint16_t

        //绑定服务器地址结构体
        if (bind(isock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        {
            close(isock);
            return false;
        }

        //开始监听
        if (listen(isock, NGX_LISTEN_BACKLOG) == -1)
        {
            close(isock);
            return false;
        }
        ngx_log_error_core(NGX_LOG_INFO, 0, "监听%d端口成功!", iport); //显示一些信息到日志中
    }
    return true;
}

//设置socket连接为非阻塞模式【这种函数的写法很固定】：非阻塞，概念在五章四节讲解的非常清楚【不断调用，不断调用这种：拷贝数据的时候是阻塞的】
bool CSocekt::setnonblocking(int sock_fd)
{
    int nb = 1; //0：清除，1：设置
    if (ioctl(sock_fd, FIONBIO, &nb) == -1) //FIONBIO：设置/清除非阻塞I/O标记：0：清除，1：设置
    {
        return false;
    }
    return true;
}

//关闭socket，什么时候用，我们现在先不确定，先把这个函数预备在这里
void CSocekt::ngx_close_listening_sockets()
{
//    for (int i = 0; i < m_ListenPortCount; i++) //要关闭这么多个监听端口
//    {
//        close(m_ListenSocketList[i]->fd);
//        ngx_log_error_core(NGX_LOG_INFO, 0, "关闭监听端口%d!", m_ListenSocketList[i]->port); //显示一些信息到日志中
//    }
}






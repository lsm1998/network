#include "network_interface.h"

Network_Interface *Network_Interface::m_network_interface = nullptr;

Network_Interface::Network_Interface() :
        epollfd_(0),
        listenfd_(0),
        websocket_handler_map_()
{
    if (0 != init())
        exit(1);
}

Network_Interface::~Network_Interface()
{

}

int Network_Interface::init()
{
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd_ == -1)
    {
        DEBUG_LOG("创建套接字失败!");
        return -1;
    }
    struct sockaddr_in server_addr{};
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    if (-1 == bind(listenfd_, (struct sockaddr *) (&server_addr), sizeof(server_addr)))
    {
        DEBUG_LOG("绑定套接字失败!");
        return -1;
    }
    if (-1 == listen(listenfd_, 5))
    {
        DEBUG_LOG("监听失败!");
        return -1;
    }
    epollfd_ = epoll_create(MAX_EVENTS_SIZE);

    ctl_event(listenfd_, true);
    DEBUG_LOG("服务器启动成功!");
    return 0;
}

int Network_Interface::epoll_loop()
{
    struct sockaddr_in client_addr{};
    socklen_t client;
    int nfds = 0;
    int fd = 0;
    struct epoll_event events[MAX_EVENTS_SIZE];
    while (true)
    {
        nfds = epoll_wait(epollfd_, events, MAX_EVENTS_SIZE, TIME_WAIT);
        if (nfds < 0)
        { break; }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listenfd_)
            {
                fd = accept(listenfd_, (struct sockaddr *) &client_addr, &client);
                ctl_event(fd, true);
            } else if (events[i].events & EPOLLIN)
            {
                if ((fd = events[i].data.fd) < 0)
                {
                    continue;
                }
                Websocket_Handler *handler = websocket_handler_map_[fd];
                if (handler == nullptr)
                {
                    continue;
                }
                if (read(fd, handler->getbuff(), BUFF_LEN) <= 0)
                {
                    ctl_event(fd, false);
                } else
                {
                    handler->process();
                }
            }
        }
    }
    return 0;
}

int Network_Interface::set_noblock(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
    {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

Network_Interface *Network_Interface::get_share_network_interface()
{
    if (m_network_interface == nullptr)
    {
        m_network_interface = new Network_Interface();
    }
    return m_network_interface;
}

void Network_Interface::ctl_event(int fd, bool flag)
{
    struct epoll_event ev{};
    ev.data.fd = fd;
    ev.events = flag ? EPOLLIN : 0;
    epoll_ctl(epollfd_, flag ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &ev);
    if (flag)
    {
        set_noblock(fd);
        websocket_handler_map_[fd] = new Websocket_Handler(fd);
        if (fd != listenfd_)
            DEBUG_LOG("fd: %d 加入epoll循环", fd);
    } else
    {
        close(fd);
        delete websocket_handler_map_[fd];
        websocket_handler_map_.erase(fd);
        DEBUG_LOG("fd: %d 退出epoll循环", fd);
    }
}

void Network_Interface::run()
{
    epoll_loop();
}

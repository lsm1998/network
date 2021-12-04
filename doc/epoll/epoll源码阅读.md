## int epoll_create(int size)

作用：创建一个epoll对象，返回该对象的描述符【文件描述符】，这个描述符就代表这个epoll对象


````C++
// epoll_create函数主要是创建eventpoll结构体
struct eventpoll {
	ep_rb_tree rbr;      //ep_rb_tree是个结构，所以rbr是结构变量，这里代表红黑树的根；
	int rbcnt;
	
	LIST_HEAD( ,epitem) rdlist;    //rdlist是结构变量，这里代表双向链表的根；
	/*	这个LIST_HEAD等价于下边这个 
		struct {
			struct epitem *lh_first;
		}rdlist;
	*/
	int rdnum; //双向链表里边的节点数量（也就是有多少个TCP连接来事件了）

	int waiting; // 代表是否在等待

	pthread_mutex_t mtx; //rbtree update
	pthread_spinlock_t lock; //rdlist update
	
	pthread_cond_t cond; //block for event
	pthread_mutex_t cdmtx; //mutex for cond
};
````

````C++
//创建epoll对象，创建一颗空红黑树，一个空双向链表
int epoll_create(int size)
{
    // size只要大于0即可
    if (size <= 0) return -1;

    tcp_manager *tcp = get_tcp_manager();
    if (!tcp) return -1;
    
    // socket对象
    struct _socket *epsocket = socket_allocate(NTY_TCP_SOCK_EPOLL);
    if (epsocket == NULL)
    {
        trace_epoll("malloc failed\n");
        return -1;
    }

    //(1)相当于new了一个eventpoll对象【开辟了一块内存】
    struct eventpoll *ep = (struct eventpoll *) calloc(1, sizeof(struct eventpoll)); //参数1：元素数量 ，参数2：每个元素大小
    if (!ep)
    {
        free_socket(epsocket->id, 0);
        return -1;
    }

    ep->rbcnt = 0;

    //(2)让红黑树根节点指向一个空
    RB_INIT(&ep->rbr);       //等价于ep->rbr.rbh_root = NULL;

    //(3)让双向链表的根节点指向一个空
    LIST_INIT(&ep->rdlist);  //等价于ep->rdlist.lh_first = NULL;
    
    /**
     * 初始化互斥量相关
     */
    if (pthread_mutex_init(&ep->mtx, NULL))
    {
        free(ep);
        free_socket(epsocket->id, 0);
        return -2;
    }

    if (pthread_mutex_init(&ep->cdmtx, NULL))
    {
        pthread_mutex_destroy(&ep->mtx);
        free(ep);
        free_socket(epsocket->id, 0);
        return -2;
    }

    if (pthread_cond_init(&ep->cond, NULL))
    {
        pthread_mutex_destroy(&ep->cdmtx);
        pthread_mutex_destroy(&ep->mtx);
        free(ep);
        free_socket(epsocket->id, 0);
        return -2;
    }

    if (pthread_spin_init(&ep->lock, PTHREAD_PROCESS_SHARED))
    {
        pthread_cond_destroy(&ep->cond);
        pthread_mutex_destroy(&ep->cdmtx);
        pthread_mutex_destroy(&ep->mtx);
        free(ep);

        free_socket(epsocket->id, 0);
        return -2;
    }

    tcp->ep = (void *) ep;
    epsocket->ep = (void *) ep;

    return epsocket->id;
}
````

## int epoll_ctl(int epid, int op, int sockid, struct epoll_event *event)

````C++
//往红黑树中加每个tcp连接以及相关的事件
int epoll_ctl(int epid, int op, int sockid, struct epoll_event *event)
{
    nty_tcp_manager *tcp = nty_get_tcp_manager();
    if (!tcp) return -1;

    nty_trace_epoll(" epoll_ctl --> 1111111:%d, sockid:%d\n", epid, sockid);
    struct _nty_socket *epsocket = tcp->fdtable->sockfds[epid];
    //struct _nty_socket *socket = tcp->fdtable->sockfds[sockid];

    //nty_trace_epoll(" epoll_ctl --> 1111111:%d, sockid:%d\n", epsocket->id, sockid);
    if (epsocket->socktype == NTY_TCP_SOCK_UNUSED)
    {
        errno = -EBADF;
        return -1;
    }

    if (epsocket->socktype != NTY_TCP_SOCK_EPOLL)
    {
        errno = -EINVAL;
        return -1;
    }

    nty_trace_epoll(" epoll_ctl --> eventpoll\n");

    struct eventpoll *ep = (struct eventpoll *) epsocket->ep;
    if (!ep || (!event && op != EPOLL_CTL_DEL))
    {
        errno = -EINVAL;
        return -1;
    }

    if (op == EPOLL_CTL_ADD) // 添加
    {
        //添加sockfd上关联的事件
        pthread_mutex_lock(&ep->mtx);

        struct epitem tmp;
        tmp.sockfd = sockid;
        struct epitem *epi = RB_FIND(_epoll_rb_socket, &ep->rbr, &tmp); //先在红黑树上找，根据key来找，也就是这个sockid，找的速度会非常快
        if (epi)
        {
            //原来有这个节点，不能再次插入
            nty_trace_epoll("rbtree is exist\n");
            pthread_mutex_unlock(&ep->mtx);
            return -1;
        }

        //只有红黑树上没有该节点【没有用过EPOLL_CTL_ADD的tcp连接才能走到这里】；

        //(1)生成了一个epitem对象，大家注意这个结构epitem，这个结构对象，其实就是红黑的一个节点，也就是说，红黑树的每个节点都是 一个epitem对象；
        epi = (struct epitem *) calloc(1, sizeof(struct epitem));
        if (!epi)
        {
            pthread_mutex_unlock(&ep->mtx);
            errno = -ENOMEM;
            return -1;
        }

        //(2)把socket(TCP连接)保存到节点中；
        epi->sockfd = sockid;  //作为红黑树节点的key，保存在红黑树中

        //(3)我们要增加的事件也保存到节点中；
        memcpy(&epi->event, event, sizeof(struct epoll_event));

        //(4)把这个节点插入到红黑树中去
        epi = RB_INSERT(_epoll_rb_socket, &ep->rbr,
                        epi); //实际上这个时候epi的rbn成员就会发挥作用，如果这个红黑树中有多个节点，那么RB_INSERT就会epi->rbi相应的值：可以参考图来理解
        assert(epi == NULL);
        ep->rbcnt++;

        pthread_mutex_unlock(&ep->mtx);

    } else if (op == EPOLL_CTL_DEL) // 删除
    {
        //把红黑树节点从红黑树上删除
        pthread_mutex_lock(&ep->mtx);

        struct epitem tmp;
        tmp.sockfd = sockid;

        struct epitem *epi = RB_FIND(_epoll_rb_socket, &ep->rbr, &tmp);//先在红黑树上找，根据key来找，也就是这个sockid，找的速度会非常快
        if (!epi)
        {
            nty_trace_epoll("rbtree no exist\n");
            pthread_mutex_unlock(&ep->mtx);
            return -1;
        }

        //只有在红黑树上找到该节点【用过EPOLL_CTL_ADD的tcp连接才能走到这里】；

        //(1)从红黑树上把这个节点干掉
        epi = RB_REMOVE(_epoll_rb_socket, &ep->rbr, epi);
        if (!epi)
        {
            nty_trace_epoll("rbtree is no exist\n");
            pthread_mutex_unlock(&ep->mtx);
            return -1;
        }

        ep->rbcnt--;
        free(epi);

        pthread_mutex_unlock(&ep->mtx);

    } else if (op == EPOLL_CTL_MOD) // 修改事件
    {
        //修改红黑树某个节点的内容
        struct epitem tmp;
        tmp.sockfd = sockid;
        struct epitem *epi = RB_FIND(_epoll_rb_socket, &ep->rbr, &tmp); //先在红黑树上找，根据key来找，也就是这个sockid，找的速度会非常快
        if (epi)
        {
            //(1)红黑树上有该节点，则修改对应的事件
            epi->event.events = event->events;
            epi->event.events |= EPOLLERR | EPOLLHUP;
        } else
        {
            errno = -ENOENT;
            return -1;
        }

    } else
    {
        nty_trace_epoll("op is no exist\n");
        assert(0);
    }

    return 0;
}
````
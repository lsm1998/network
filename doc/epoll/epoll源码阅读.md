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
// 红黑树的元素
struct epitem {
	RB_ENTRY(epitem) rbn;
	/*  RB_ENTRY相当如定义了如下的一个结构成员变量
	struct {											
	struct type *rbe_left;		//指向左子树
	struct type *rbe_right;		//指向右子树
	struct type *rbe_parent;	//指向父节点
	int rbe_color;			    //该红黑树节点颜色
	} rbn*/

	LIST_ENTRY(epitem) rdlink;
	/*
	struct {									
		struct type *le_next;	//指向下个元素
		struct type **le_prev;	//前一个元素的地址
	}*/

	int rdy; //exist in list 是否这个节点是同时在双向链表中【这个节点刚开始是在红黑树中】
	
	int sockfd;
	struct epoll_event event; 
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
        // sockid作为key
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

## int epoll_wait(int epid, struct epoll_event *events, int maxevents, int timeout)
````C++
//到双向链表中去取相关的事件通知
int epoll_wait(int epid, struct epoll_event *events, int maxevents, int timeout)
{

    nty_tcp_manager *tcp = nty_get_tcp_manager();
    if (!tcp) return -1;

    //nty_socket_map *epsocket = &tcp->smap[epid];
    struct _nty_socket *epsocket = tcp->fdtable->sockfds[epid];
    if (epsocket == NULL) return -1;

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

    struct eventpoll *ep = (struct eventpoll *) epsocket->ep;
    if (!ep || !events || maxevents <= 0)
    {
        errno = -EINVAL;
        return -1;
    }

    if (pthread_mutex_lock(&ep->cdmtx))
    {
        if (errno == EDEADLK)
        {
            nty_trace_epoll("epoll lock blocked\n");
        }
        assert(0);
    }

    //(1)这个while用来等待一定的时间【在这段时间内，发生事件的TCP连接，相关的节点，会被操作系统扔到双向链表去【当然这个节点同时也在红黑树中呢】】
    while (ep->rdnum == 0 && timeout != 0)
    {

        ep->waiting = 1;
        if (timeout > 0)
        {

            struct timespec deadline;

            clock_gettime(CLOCK_REALTIME, &deadline);
            if (timeout >= 1000)
            {
                int sec;
                sec = timeout / 1000;
                deadline.tv_sec += sec;
                timeout -= sec * 1000;
            }

            deadline.tv_nsec += timeout * 1000000;

            if (deadline.tv_nsec >= 1000000000)
            {
                deadline.tv_sec++;
                deadline.tv_nsec -= 1000000000;
            }

            int ret = pthread_cond_timedwait(&ep->cond, &ep->cdmtx, &deadline);
            if (ret && ret != ETIMEDOUT)
            {
                nty_trace_epoll("pthread_cond_timewait\n");

                pthread_mutex_unlock(&ep->cdmtx);

                return -1;
            }
            timeout = 0;
        } else if (timeout < 0)
        {

            int ret = pthread_cond_wait(&ep->cond, &ep->cdmtx);
            if (ret)
            {
                nty_trace_epoll("pthread_cond_wait\n");
                pthread_mutex_unlock(&ep->cdmtx);

                return -1;
            }
        }
        ep->waiting = 0;

    }

    pthread_mutex_unlock(&ep->cdmtx);

    //等一小段时间，等时间到达后，流程来到这里。。。。。。。。。。。。。。

    pthread_spin_lock(&ep->lock);

    int cnt = 0;

    //(1)取得事件的数量
    //ep->rdnum：代表双向链表里边的节点数量（也就是有多少个TCP连接来事件了）
    //maxevents：此次调用最多可以收集到maxevents个已经就绪【已经准备好】的读写事件
    int num = (ep->rdnum > maxevents ? maxevents : ep->rdnum); //哪个数量少，就取得少的数字作为要取的事件数量
    int i = 0;

    while (num != 0 && !LIST_EMPTY(&ep->rdlist))
    { //EPOLLET

        //(2)每次都从双向链表头取得 一个一个的节点
        struct epitem *epi = LIST_FIRST(&ep->rdlist);

        //(3)把这个节点从双向链表中删除【但这并不影响这个节点依旧在红黑树中】
        LIST_REMOVE(epi, rdlink);

        //(4)这是个标记，标记这个节点【这个节点本身是已经在红黑树中】已经不在双向链表中；
        epi->rdy = 0;  //当这个节点被操作系统 加入到 双向链表中时，这个标记会设置为1。

        //(5)把事件标记信息拷贝出来；拷贝到提供的events参数中
        memcpy(&events[i++], &epi->event, sizeof(struct epoll_event));

        num--;
        cnt++;       //拷贝 出来的 双向链表 中节点数目累加
        ep->rdnum--; //双向链表里边的节点数量减1
    }

    pthread_spin_unlock(&ep->lock);

    //(5)返回 实际 发生事件的 tcp连接的数目；
    return cnt;
}
````

## epoll的双向链表数据来源

由操作系统内核回调epoll_event_callback函数将事件写入链表；

以下4种情况：
1. 客户端完成三次握手，服务端需要accept；
2. 客户端主动关闭连接，服务的需要close；
3. 客户端发送数据，服务端需要read、recv；
4. 当可以发送数据时，服务端需要write、send；
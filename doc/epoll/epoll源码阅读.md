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


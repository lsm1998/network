//
// Created by Administrator on 2021/11/2.
//
#include "icmp.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <cerrno>

#define MAX_WAIT_TIME 5
#define PACKET_SIZE 4096    /* 数据包的大小 */
#define MAX_NO_PACKETS 3    /* 发送3个ICMP报文 */

char sendpacket[PACKET_SIZE];    /* 发送的数据包 */
char recvpacket[PACKET_SIZE];    /* 接收的数据包 */

pid_t pid;
int sockfd;
int datalen = 56;    /* icmp数据包中数据的长度 */
int nsend = 0;       /* 发送的次数 */
int nreceived = 0;   /* 接收的次数 */

struct sockaddr_in dest_addr;  /* icmp包目的地址 */
struct sockaddr_in from;       /* icmp包源地址 */
struct timeval tvrecv;

void statistics(int signo);

unsigned short cal_chksum(unsigned short *addr, int len);

int pack(int pack_no);

void send_packet(void);

void recv_packet(void);

int unpack(char *buf, int len);

void tv_sub(struct timeval *out, struct timeval *in);

void statistics(int signo)
{
    printf("\n--------------------PING statistics-------------------\n");
    /*
     * 总共发送nsend个icmp包，总共接收到返回的nreceived个包，
     * icmp包的丢失率(nsend-nreceived)/nsend
     */
    printf("%d packets transmitted, %d received , %%%d lost\n",
           nsend, nreceived, (nsend - nreceived) / nsend * 100);
    close(sockfd);
    exit(1);
}

/* 计算校验和的算法 */
unsigned short cal_chksum(unsigned short *addr, int len)
{
    int sum = 0;
    int nleft = len;
    unsigned short *w = addr;
    unsigned short answer = 0;

    /* 把ICMP报头二进制数据以2字节为单位累加起来 */
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    /*
     * 若ICMP报头为奇数个字节，会剩下最后一字节。
     * 把最后一个字节视为一个2字节数据的高字节，
     * 这2字节数据的低字节为0，继续累加
     */
    if (nleft == 1)
    {
        *(unsigned char *) (&answer) = *(unsigned char *) w;
        sum += answer;    /* 这里将 answer 转换成 int 整数 */
    }
    sum = (sum >> 16) + (sum & 0xffff);        /* 高位低位相加 */
    sum += (sum >> 16);        /* 上一步溢出时，将溢出位也加到sum中 */
    answer = ~sum;             /* 注意类型转换，现在的校验和为16位 */

    return answer;
}

/* 设置ICMP报头，以及将发送的时间设置为ICMP的末尾的数据部分和校验和 */
int pack(int pack_no)
{
    int packsize;
    struct icmp *icmp;
    struct timeval *tval;

    icmp = (struct icmp *) sendpacket;
    icmp->icmp_type = ICMP_ECHO;    /* icmp的类型 */
    icmp->icmp_code = 0;            /* icmp的编码 */
    icmp->icmp_cksum = 0;           /* icmp的校验和 */
    icmp->icmp_seq = pack_no;       /* icmp的顺序号 */
    icmp->icmp_id = pid;            /* icmp的标志符 */
    packsize = 8 + datalen;   /* icmp8字节的头 加上数据的长度(datalen=56), packsize = 64 */

    tval = (struct timeval *) icmp->icmp_data;    /* 获得icmp结构中最后的数据部分的指针 */
    gettimeofday(tval, NULL); /* 将发送的时间填入icmp结构中最后的数据部分 */

    icmp->icmp_cksum = cal_chksum((unsigned short *) icmp, packsize);/*填充发送方的校验和*/

    return packsize;
}

/* 发送三个ICMP报文 */
void send_packet()
{
    int packetsize;

    /* 每一次发送3个icmp包 */
    while (nsend < MAX_NO_PACKETS)
    {    // #define MAX_NO_PACKETS 3
        nsend++;
        packetsize = pack(nsend); /* 设置ICMP报头 */
        if (sendto(sockfd, sendpacket, packetsize, 0,
                   (struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0)
        {
            perror("sendto error");
            continue;
        }
        sleep(1); /* 每隔一秒发送一个ICMP报文 */
    }
}

/* 接收所有ICMP报文 */
void recv_packet()
{
    int n, fromlen;

    signal(SIGALRM, statistics);
    fromlen = sizeof(from);        /* icmp包源地址的大小*/

    while (nreceived < nsend)
    {
        alarm(MAX_WAIT_TIME);
        if ((n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0,
                          (struct sockaddr *) &from, (socklen_t *) &fromlen)) < 0)
        {
            if (errno == EINTR)
                continue;
            perror("recvfrom error");
            continue;
        }
        gettimeofday(&tvrecv, nullptr); /* 记录接收到icmp包时的时间 */
        if (unpack(recvpacket, n) == -1)
            continue;
        nreceived++;
    }
}

/* 对ICMP报头解包 */
int unpack(char *buf, int len)
{
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend;
    double rtt;

    ip = (struct ip *) buf;
    iphdrlen = ip->ip_hl << 2;    /* 求ip报头长度,即ip报头的长度标志乘4 */

    icmp = (struct icmp *) (buf + iphdrlen); /* 越过ip报头,指向ICMP报头 */
    len -= iphdrlen;        /* ICMP报头及ICMP数据报的总长度 */
    if (len < 8)
    {                /* 小于ICMP报头长度则不合理 */
        printf("ICMP packets\'s length is less than 8\n");
        return -1;
    }
    /* 确保所接收的是我所发的的ICMP的回应 */
    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid))
    {
        tvsend = (struct timeval *) icmp->icmp_data;
        tv_sub(&tvrecv, tvsend);   /* 接收和发送的时间差 */
        /* 以毫秒为单位计算发送和接收的时间差rtt */
        rtt = tvrecv.tv_sec * 1000 + tvrecv.tv_usec / 1000;
        /* 显示相关信息 */
        printf("%d byte from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
               len,        /* ICMP报头及ICMP数据报的总长度 */
               inet_ntoa(from.sin_addr),    /* ICMP的源地址 */
               icmp->icmp_seq,        /* icmp包发送的顺序 */
               ip->ip_ttl,            /* icmp存活的时间 */
               rtt);        /* 以毫秒为单位计算发送和接收的时间差rtt */
        return 0;
    } else
        return -1;
}

int main(int argc, char *argv[])
{
    struct hostent *host;
    struct protoent *protocol;
    unsigned long inaddr = 0l;
    int size = 50 * 1024;        //50k

    if (argc < 2)
    {
        printf("usage:%s hostname/IP address\n", argv[0]);
        exit(1);
    }
    if ((protocol = getprotobyname("icmp")) == nullptr)
    {
        perror("getprotobyname");
        exit(1);
    }
    /* 生成使用ICMP的原始套接字,这种套接字只有root才能生成 */
    if ((sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    setuid(getuid());    /* 回收root权限,设置当前用户权限 */

    /*
     * 扩大套接字接收缓冲区到50K这样做主要为了减小接收缓冲区溢出的
     * 的可能性,若无意中ping一个广播地址或多播地址,将会引来大量应答
     */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

    /* 判断argv[1]是主机名还是ip地址 */
    if ((inaddr = inet_addr(argv[1])) == INADDR_NONE)
    {
        if ((host = gethostbyname(argv[1])) == nullptr)
        {    /* 是主机名 */
            perror("gethostbyname error");
            exit(1);
        }
        memcpy((char *) &dest_addr.sin_addr, host->h_addr, host->h_length);
    } else
    {/* 是ip地址 */
        memcpy((char *) &dest_addr.sin_addr, (char *) &inaddr, sizeof(inaddr));
    }
    pid = getpid();        /*获取main的进程id,用于设置ICMP的标志符*/
    printf("PING %s(%s): %d bytes data in ICMP packets.\n",
           argv[1], inet_ntoa(dest_addr.sin_addr), datalen);

    send_packet();     /* 发送所有ICMP报文 */
    recv_packet();     /* 接收所有ICMP报文 */
    statistics(SIGALRM);     /* 进行统计 */

    return 0;
}

/* 两个timeval结构相减 */
void tv_sub(struct timeval *recv, struct timeval *send)
{
    if ((recv->tv_usec -= send->tv_usec) < 0)
    {
        --recv->tv_sec;
        recv->tv_usec += 1000000;
    }
    recv->tv_sec -= send->tv_sec;
}

//
// Created by Administrator on 2021/11/2.
//

#ifndef NETWORK_ICMP_H
#define NETWORK_ICMP_H

#include <cstdint>
#include <stdbool.h>

/**
ICMPv4 Header
*/
typedef struct ICMPHeader
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
} ICMPHeader;

/**
Destination Unreachable Message "目的不可达消息格式"
type:消息类型，此处值为3。
Code:0 = net unreachable;网络不可达
     1 = host unreachable;主机不可达
     2 = protocol unreachable;协议不可达
     3 = port unreachable; 端口不可达，Tracert时发送的ICMP报文即为此类。
     4 = fragmentation needed and DF set;需要进行分片但设置不分片比特
     5 = source route failed.源站选路失败
     6 = Destination network unknown目的网络不认识
     7 = Destination host unknown目的主机不认识
     8 = Source host isolated (obsolete)源主机被隔离（作废不用）
     9 = Destination network administratively prohibited目的网络被强制禁止
     10 = Destination host administratively prohibited目的主机被强制禁止
     11 = Network unreachable for TOS由于TOS，网络不可达
     12 = Host unreachable for TOS 由于TOS，主机不可达
     13 = Communication administratively prohibited by filtering由于过滤，通信被强制禁止
     14 = Host precedence violation主机越权
     15 = Precedence cutoff in effect优先权中止生效
checkSum:检验和，使用和IP相同的加法校验和算法，但是ICMP校验和仅覆盖ICMP报文。
unused:4字节，未使用，必须填0。
data: Internet Header + 64 bits of Original Data Datagram
    IP首部+原始数据包的前8字节：
        1.IP首部：如果IP首部没有选项字段时为20字节
        2.原始数据包的前8字节：UDP首部的8字节或者TCP首部的8字节。
          该数据是主机用来匹配消息。对于更高层协议的用户端口号，原始数据包的前64比特的这些数据会被重组。
*/
#define ICMP_Type_Destination_Unreachable 3
//
#define ICMP_Code_Net_Unreachable 0
#define ICMP_Code_Host_Unreachable 1
#define ICMP_Code_Protocol_Unreachable 2
#define ICMP_Code_Port_Unreachable 3
#define ICMP_Code_Fragmentation_Needed_And_DF_Set 4
#define ICMP_Code_Source_Route_Failed 5

#define ICMP_Code_Destination_Network_Unknown 6
#define ICMP_Code_Destination_Host_Unknown 7
#define ICMP_Code_Source_Host_Isolated 8
#define ICMP_Code_Destination_Network_Administratively_Prohibited 9
#define ICMP_Code_Destination_Host_Administratively_Prohibited 10
#define ICMP_Code_Network_Unreachable_For_TOS 11
#define ICMP_Code_Host_Unreachable_For_TOS 12
#define ICMP_Code_Communication_AdministrativelyProhibited_By_Filtering 13
#define ICMP_Code_Host_Precedence_Violation 14
#define ICMP_Code_Precedence_Cutoff_In_Effect 15
typedef struct ICMPDestinationUnreachable
{
    struct ICMPHeader iCMPHeader;
    uint32_t unused;
    uint8_t data[];
} ICMPDestinationUnreachable;

/**
Time Exceeded Message "超时消息格式"
type:消息类型，此处值为3。
code:0 = time to live exceeded in transit
     1 = fragment reassembly time exceeded
checkSum:检验和，使用和IP相同的加法校验和算法，但是ICMP校验和仅覆盖ICMP报文。
unused:4字节，未使用，必须填0。
data: Internet Header + 64 bits of Original Data Datagram
    IP首部+原始数据包的前8字节：
        1.IP首部：如果IP首部没有选项字段时为20字节
        2.原始数据包的前8字节：UDP首部的8字节或者TCP首部的8字节。
          该数据是主机用来匹配消息。对于更高层协议的用户端口号，原始数据包的前64比特的这些数据会被重组。
*/
#define ICMP_Type_Time_Exceeded 3
//
#define ICMP_Code_Time_To_Live_Exceeded_InTransit 0
#define ICMP_Code_Fragment_Reassembly_Time_Exceeded 1
typedef struct ICMPTimeExceeded
{
    struct ICMPHeader iCMPHeader;
    uint32_t unused;
    uint8_t data[];
} ICMPTimeExceeded;

/**
Parameter Problem Message
type: 12
code: 0 = pointer indicates the error.
*/
#define ICMP_Type_Parameter_Problem 12
//
#define ICMP_Code_Pointer_Indicates_The_Error 0
typedef struct ICMPParameterProblem
{
    struct ICMPHeader iCMPHeader;
    uint8_t pointer;
    uint32_t unused: 24;
    uint8_t data[];
} ICMPParameterProblem;

/**
Source Quench Message
type:4
code:0
*/
#define ICMP_Type_Quench_Message 4
//
#define ICMP_Code_Quench_Message 0
typedef struct ICMPSourceQuench
{
    struct ICMPHeader iCMPHeader;
    uint32_t unused;
    uint8_t data[];
} ICMPSourceQuench;

/**
Redirect Message "重定向消息"
type:消息类型，此处值为5。
Code:0 = Redirect datagrams for the Network
     1 = Redirect datagrams for the Host.
     2 = Redirect datagrams for the Type of Service and Network.
     3 = Redirect datagrams for the Type of Service and Host.
checkSum:校验和，使用和IP相同的加法校验和算法，但是ICMP校验和仅覆盖ICMP报文。
Gateway Internet Address:即原始数据包里的IP目的地址域。
data:Internet Header + 64 bits of Original Data Datagram
      IP头和原始数据包的前64比特数据。该数据是主机用来匹配消息。
      对于更高层协议的用户端口号，原始数据包的前64比特的这些数据会被重组。
*/
#define ICMP_Type_Redirect 5
//
#define ICMP_Code_Redirect_Datagrams_For_The_Network 0
#define ICMP_Code_Redirect_Datagrams_For_The_Host 1
#define ICMP_Code_Redirect_Datagrams_For_The_Type_Of_Service_And_Network 2
#define ICMP_Code_Redirect_Datagrams_For_The_Type_Of_Service_And_Host 3
typedef struct ICMPRedirect
{
    struct ICMPHeader iCMPHeader;
    uint32_t gatewayInternetAddress;
    uint8_t data[];
} ICMPRedirect;

/**
Echo or Echo Reply Message   "Echo Request/Reply消息格式"
type:1字节 0:回显应答报文
           8:请求回显报文
Code:1字节 消息代码，此处值为0。
checkSum:2字节，校验和，使用和IP相同的加法校验和算法，但是ICMP校验和仅覆盖ICMP报文。
identifier:2字节，标识符，发送端标示此发送的报文。标识符可以像TCP或UDP中的端口一样使用
SequenceNumber:2字节，序列号，发送端发送的报文的顺序号。每发送一次顺序号就加1。
data:可变，字段的长度和内容，取决于消息的类型和代码，请参见表1。
*/
#define ICMP_Type_Echo_Request 8
#define ICMP_Type_Echo_Reply 0
//
#define ICMP_Code_Echo 0
typedef struct ICMPEcho
{
    struct ICMPHeader iCMPHeader;
    uint16_t identifier;
    uint16_t sequenceNumber;
    uint8_t data[];
} ICMPEcho;

/**
uint16_t calculateICMPCheckSum(uint16_t *icmp,int size)
*/
uint16_t calculateICMPCheckSum(uint16_t *icmp);

/**
uint16_t validateICMPCheckSum(uint16_t *icmp,int size)
*/
bool validateICMPCheckSum(uint16_t *icmp);

/**
calculateICMPSize()
*/
int calculateICMPSize(void *icmp);

#endif //NETWORK_ICMP_H

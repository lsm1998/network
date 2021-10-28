//
// Created by Administrator on 2021/10/24.
//

#include "log.h"

ngx_log_t ngx_log = {};


void DEBUG()
{

}

void ERROR()
{}

void INFO()
{}

u_char *ngx_cpymem(void *dst, const void *src, size_t n)
{
    return (u_char *) memcpy(dst, src, n) + n;
}

void log_stderr(int err, const char *fmt, ...)
{
    va_list args;                        //创建一个va_list类型变量
    u_char errstr[NGX_MAX_ERROR_STR + 1]; //2048  -- ************  +1是我自己填的，感谢官方写法有点小瑕疵，所以动手调整一下
    u_char *p, *last;

    memset(errstr, 0, sizeof(errstr));     //我个人加的，这块有必要加，至少在va_end处理之前有必要，否则字符串没有结束标记不行的；***************************

    last = errstr + NGX_MAX_ERROR_STR;        //last指向整个buffer最后去了【指向最后一个有效位置的后面也就是非有效位】，作为一个标记，防止输出内容超过这么长,
    //其实我认为这有问题，所以我才在上边errstr[NGX_MAX_ERROR_STR+1]; 给加了1
    //比如你定义 char tmp[2]; 你如果last = tmp+2，那么last实际指向了tmp[2]，而tmp[2]在使用中是无效的

    p = ngx_cpymem(errstr, "nginx: ", 7);     //p指向"nginx: "之后

    va_start(args, fmt); //使args指向起始的参数
    p = vslprintf(p, last, fmt, args); //组合出这个字符串保存在errstr里
    va_end(args);        //释放args

    if (err)  //如果错误代码不是0，表示有错误发生
    {
        //错误代码和错误信息也要显示出来
        p = ngx_log_errno(p, last, err);
    }

    //若位置不够，那换行也要硬插入到末尾，哪怕覆盖到其他内容
    if (p >= (last - 1))
    {
        p = (last - 1) - 1; //把尾部空格留出来，这里感觉nginx处理的似乎就不对
        //我觉得，last-1，才是最后 一个而有效的内存，而这个位置要保存\0，所以我认为再减1，这个位置，才适合保存\n
    }
    *p++ = '\n'; //增加个换行符

    //往标准错误【一般是屏幕】输出信息
    write(STDERR_FILENO, errstr, p - errstr); //三章七节讲过，这个叫标准错误，一般指屏幕

    if (ngx_log.fd > STDERR_FILENO) //如果这是个有效的日志文件，本条件肯定成立，此时也才有意义将这个信息写到日志文件
    {
        //因为上边已经把err信息显示出来了，所以这里就不要显示了，否则显示重复了
        err = 0;    //不要再次把错误信息弄到字符串里，否则字符串里重复了
        p--;
        *p = 0; //把原来末尾的\n干掉，因为到ngx_log_err_core中还会加这个\n
        ngx_log_error_core(NGX_LOG_STDERR, err, (const char *) errstr);
    }
}

u_char *ngx_log_errno(unsigned char *buf, const unsigned char *last, int err)
{
    //以下代码是我自己改造，感觉作者的代码有些瑕疵
    char *perrorinfo = strerror(err); //根据资料不会返回NULL;
    size_t len = strlen(perrorinfo);

    //然后我还要插入一些字符串： (%d:)
    char leftstr[10] = {0};
    sprintf(leftstr, " (%d: ", err);
    size_t leftlen = strlen(leftstr);

    char rightstr[] = ") ";
    size_t rightlen = strlen(rightstr);

    size_t extralen = leftlen + rightlen; //左右的额外宽度
    if ((buf + len + extralen) < last)
    {
        //保证整个我装得下，我就装，否则我全部抛弃 ,nginx的做法是 如果位置不够，就硬留出50个位置【哪怕覆盖掉以往的有效内容】，也要硬往后边塞，这样当然也可以；
        buf = ngx_cpymem(buf, leftstr, leftlen);
        buf = ngx_cpymem(buf, perrorinfo, len);
        buf = ngx_cpymem(buf, rightstr, rightlen);
    }
    return buf;
}

void ngx_log_error_core(int level, int err, const char *fmt, ...)
{
    u_char *last;
    u_char errstr[NGX_MAX_ERROR_STR + 1];   //这个+1也是我放入进来的，本函数可以参考ngx_log_stderr()函数的写法；

    memset(errstr, 0, sizeof(errstr));
    last = errstr + NGX_MAX_ERROR_STR;

    struct timeval tv{};
    struct tm tm{};
    time_t sec;   //秒
    u_char *p;    //指向当前要拷贝数据到其中的内存位置
    va_list args;

    memset(&tv, 0, sizeof(struct timeval));
    memset(&tm, 0, sizeof(struct tm));

    gettimeofday(&tv, nullptr);     //获取当前时间，返回自1970-01-01 00:00:00到现在经历的秒数【第二个参数是时区，一般不关心】

    sec = tv.tv_sec;             //秒
    localtime_r(&sec, &tm);      //把参数1的time_t转换为本地时间，保存到参数2中去，带_r的是线程安全的版本，尽量使用
    tm.tm_mon++;                 //月份要调整下正常
    tm.tm_year += 1900;          //年份要调整下才正常

    u_char strcurrtime[40] = {0};  //先组合出一个当前时间字符串，格式形如：2019/01/08 19:57:11
    slprintf(strcurrtime,
             (u_char *) -1,                       //若用一个u_char *接一个 (u_char *)-1,则 得到的结果是 0xffffffff....，这个值足够大
             "%4d/%02d/%02d %02d:%02d:%02d",     //格式是 年/月/日 时:分:秒
             tm.tm_year, tm.tm_mon,
             tm.tm_mday, tm.tm_hour,
             tm.tm_min, tm.tm_sec);
    p = ngx_cpymem(errstr, strcurrtime, strlen((const char *) strcurrtime));  //日期增加进来，得到形如：     2019/01/08 20:26:07
    p = slprintf(p, last, " [%s] ", err_levels[level]);                //日志级别增加进来，得到形如：  2019/01/08 20:26:07 [crit]
    p = slprintf(p, last, "%P: ",
                 nginx_pid);                             //支持%P格式，进程id增加进来，得到形如：   2019/01/08 20:50:15 [crit] 2037:

    va_start(args, fmt);                     //使args指向起始的参数
    p = vslprintf(p, last, fmt, args);   //把fmt和args参数弄进去，组合出来这个字符串
    va_end(args);                            //释放args

    if (err)  //如果错误代码不是0，表示有错误发生
    {
        //错误代码和错误信息也要显示出来
        p = ngx_log_errno(p, last, err);
    }
    //若位置不够，那换行也要硬插入到末尾，哪怕覆盖到其他内容
    if (p >= (last - 1))
    {
        p = (last - 1) - 1; //把尾部空格留出来，这里感觉nginx处理的似乎就不对
        //我觉得，last-1，才是最后 一个而有效的内存，而这个位置要保存\0，所以我认为再减1，这个位置，才适合保存\n
    }
    *p++ = '\n'; //增加个换行符

    //这么写代码是图方便：随时可以把流程弄到while后边去；大家可以借鉴一下这种写法
    ssize_t n;
    while (true)
    {
        if (level > ngx_log.log_level)
        {
            //要打印的这个日志的等级太落后（等级数字太大，比配置文件中的数字大)
            //这种日志就不打印了
            break;
        }
        //磁盘是否满了的判断，先算了吧，还是由管理员保证这个事情吧；

        //写日志文件
        n = write(ngx_log.fd, errstr, p - errstr);  //文件写入成功后，如果中途
        if (n == -1)
        {
            //写失败有问题
            if (errno == ENOSPC) //写失败，且原因是磁盘没空间了
            {
                //磁盘没空间了
                //没空间还写个毛线啊
                //先do nothing吧；
            } else
            {
                //这是有其他错误，那么我考虑把这个错误显示到标准错误设备吧；
                if (ngx_log.fd != STDERR_FILENO) //当前是定位到文件的，则条件成立
                {
                    n = write(STDERR_FILENO, errstr, p - errstr);
                }
            }
        }
        break;
    } //end while
}
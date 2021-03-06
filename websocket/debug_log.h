#ifndef __debug_log__
#define __debug_log__

#include <cstdio>
#include <ctime>
#include <cstring>
#include <cstdarg>

#define __WRITE_FILE__ 1

void DEBUG_LOG(const char *msg, ...);

class Debug_LOG
{
private:
    Debug_LOG();

    ~Debug_LOG();

    void create_log_file();

public:
    static Debug_LOG *log();

    void write_log(const char *msg);

private:
    static Debug_LOG *m_log;
    time_t tim;
    struct tm *t;
    FILE *fp;
    char filepath[32];
    char message[256];
    struct tm last_log_time;
};

#endif

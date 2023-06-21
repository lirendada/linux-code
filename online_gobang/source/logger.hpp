#ifndef __MY_LOG__
#define __MY_LOG__
#include <stdio.h>
#include <time.h>

#define INF 0    // 提示型等级
#define DEBUG 1  // 调试型等级
#define ERROR 2  // 错误型等级
#define DEFAULT_LOG_LEVEL DEBUG  // 默认的日志等级

#define LOG(level, format, ...) do{\
    if(DEFAULT_LOG_LEVEL < level) break;\
    char timebuffer[128] = {0};\
    time_t timestamp = time(NULL);\
    struct tm* timeinfo = localtime(&timestamp);\
    strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", timeinfo);\
    fprintf(stdout, "[%s %s:%d] " format "\n", timebuffer, __FILE__, __LINE__, ##__VA_ARGS__);\
}while(0)

// 将等级和日志打印封装起来
#define ILOG(format, ...) LOG(INF, format, ##__VA_ARGS__)
#define DLOG(format, ...) LOG(DEBUG, format, ##__VA_ARGS__)
#define ELOG(format, ...) LOG(ERROR, format, ##__VA_ARGS__)

#endif
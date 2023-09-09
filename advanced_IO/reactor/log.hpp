#pragma once
#include <iostream>
#include <string>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
using namespace std;

const int NUM = 1024;
enum Level{
    DEBUG = 0,
    NORMAL,
    WARING, 
    ERROR,
    FATAL
};
const char* to_levelstr(int level)
{
    switch(level)
    {
        case DEBUG: return "DEBUG";
        case NORMAL: return "NORMAL";
        case WARING: return "WARING";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default: return nullptr;
    }
}

// 日志格式：[日志等级][时间戳/时间][pid][message]
void logMessage(int level, const char* format, ...)
{
    // 1. 先将时间戳转化为本地时间然后格式化
    char timebuffer[128];
    time_t timestamp = time(nullptr); 			 // 获取当前时间戳
    struct tm* timeinfo = localtime(&timestamp); // 转化为本地时间结构
    strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", timeinfo); // 格式化时间字符串

    // 2. 拼凑前缀部分，是固定的
    char prefixbuffer[NUM];
    snprintf(prefixbuffer, sizeof(prefixbuffer), "[%s][%s][%d]", to_levelstr(level), timebuffer, getpid());

    // 3. 格式化信息部分也就是后缀部分，是可变参数的内容 -- 通过vsnprintf格式化到数组中
    char msgbuffer[NUM];
    va_list start;
    va_start(start, format);
    vsnprintf(msgbuffer, sizeof(msgbuffer), format, start);

    // 4. 打印
    printf("%s%s\n", prefixbuffer, msgbuffer);
}
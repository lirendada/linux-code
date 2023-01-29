#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_SIZE 1024

#define SYNC_NOW  1  // 直接刷新
#define SYNC_LINE 2  // 行缓冲
#define SYNC_FULL 4  // 全缓冲

typedef struct _FILE
{
    int flags; // 缓冲方式
    int fileno; // 文件描述符
    char buffer[MAX_SIZE]; // 缓冲区
    int size; // buffer的有效个数
    int cap; // buffer的总容量
}_FILE;

_FILE* _fopen(const char* path_name, const char* mode);
void _fwrite(_FILE* fp, const void* ptr, int num);
void _fclose(_FILE* fp);
void _fflush(_FILE* fp);
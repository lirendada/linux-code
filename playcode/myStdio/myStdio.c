#include "myStdio.h"

_FILE* _fopen(const char* path_name, const char* mode)
{
    int flag = 0; // 标记打开方式
    if(strcmp(mode, "r") == 0)
    {
        flag |= O_RDONLY;
    }
    else if(strcmp(mode, "w") == 0)
    {
        flag |= (O_WRONLY | O_CREAT | O_TRUNC);
    }
    else if(strcmp(mode, "a") == 0)
    {
        flag |= (O_WRONLY | O_CREAT | O_APPEND);
    }

    int fd = 0;
    int defaultMode = 0666;
    // 选择打开方式
    if(flag & O_RDONLY)
        fd = open(path_name, flag);
    else 
        fd = open(path_name, flag, defaultMode);

    if(fd < 0)
    {
        const char* err = strerror(errno);
        write(2, err, strlen(err));

        // 返回null，这也就是为什么我们自己调用fopen的时候失败返回null
        return NULL;
    }

    // 为结构体开辟空间
    _FILE* fp = (_FILE*)malloc(sizeof(_FILE));
    assert(fp);

    fp->flags = SYNC_LINE; // 默认设置为行刷新
    fp->fileno = fd;
    fp->cap = MAX_SIZE;
    fp->size = 0;
    memset(fp->buffer, 0, MAX_SIZE); // 将buffer设为0

    return fp; // 这就是为什么打开文件就返回一个文件指针
}

void _fwrite(_FILE* fp, const void* ptr, int num)
{
    // 1、将数据写到缓冲区内，而不是写到操作系统内
    // 这里不考虑缓冲区溢出的问题
    memcpy(fp->buffer + fp->size, ptr, num);
    fp->size += num;

    // 2、判断是否需要刷新
    if(fp->flags & SYNC_NOW)
    {
        _fflush(fp);
    }
    else if(fp->flags & SYNC_LINE)
    {
        if(fp->buffer[fp->size - 1] == '\n')
        {
            _fflush(fp);
        }
    }
    else if(fp->flags & SYNC_FULL)
    {
        if(fp->size == fp->cap)
        {
            _fflush(fp);
        }
    }
}

void _fflush(_FILE* fp)
{
    if(fp->size > 0)
    {
        write(fp->fileno, fp->buffer, fp->size);
        fp->size = 0; // 记得清空有效个数
    }
}

void _fclose(_FILE* fp)
{
    // 刷新缓冲区，并关闭
    _fflush(fp);
    close(fp->fileno);
}
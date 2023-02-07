#pragma once

#include <iostream>
#include <string>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define NAMED_PIPE "/tmp/my_named_pipe"    // 存放到系统中的共享目录下

bool createFifo(const std::string& path)
{
    umask(0);
    int n = mkfifo(path.c_str(), 0666); // 创建管道文件
    if(n == 0)
        return true;
    else 
    {
        std::cout << "errno: " << errno << " err string: " << strerror(errno) << std::endl;
        return false;
    }
}

void removeFifo(const std::string& path)
{
    int n = unlink(path.c_str()); // 移除管道文件目录项
    assert(n == 0);
    (void)n;
}
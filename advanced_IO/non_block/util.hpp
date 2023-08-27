#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

void DownLoad()
{
    std::cout << "DownLoad" << std::endl;
}

void Print()
{
    std::cout << "Print" << std::endl;
}

void ExecuteSql()
{
    std::cout << "ExecuteSql" << std::endl;
}

void setNonBlock(int fd)
{
    int flag = fcntl(fd, F_GETFL); // 获取文件描述符的标记
    if(flag < 0)
    {
        std::cerr << "fcntl: " << strerror(errno) << std::endl;
        return;
    }

    // 设置文件描述符的标记为非阻塞
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
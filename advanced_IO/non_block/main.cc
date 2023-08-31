#include <cstdio>
#include "util.hpp"

#define RUN(callbacks) do{\
    callbacks();\
}while(0)

int main()
{
    // 设置非阻塞等待
    setNonBlock(0);

    char buffer[1024]; 
    while(true)
    {
        printf(">>> ");
        fflush(stdout); 

        ssize_t n = read(0, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
            buffer[n - 1] = 0;
            std::cout << "echo: " << buffer << std::endl;
        }
        else if(n == 0)
        {
            std::cout << "read end" << std::endl;
            break;
        }
        else
        {
            // std::cout << "errno: " << errno << ", is " << strerror(errno) << std::endl;
            // std::cout << "EAGAIN: " << EAGAIN << "  EWOULDBLOCK: " << EWOULDBLOCK << std::endl;

            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cout << "未输入数据，并不是读取错误！" << std::endl;
            }
            else if(errno == EINTR)
            {
                std::cout << "读取被中断，需要继续上一次的读取！" << std::endl;
                continue;
            }
            else
            {
                std::cout << "读取错误！" << std::endl;
                break;
            }
        }

        RUN(DownLoad);
        std::cout << "---------------------------------------" << std::endl;
        sleep(2); 
    }
    return 0;
}
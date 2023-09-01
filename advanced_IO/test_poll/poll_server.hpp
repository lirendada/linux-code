#pragma once
#include <iostream>
#include <poll.h>
#include "sock.hpp"

namespace poll_space
{
    const static int default_port = 8080;    // 服务器默认端口号
    const static int free_num = -1;          // 监听集合中空闲位置的默认值
    const static nfds_t pollfd_size = 4096;  // pollfd数组的长度

    class poll_server
    {
    private:
        int _listensock;
        int _port;
        struct pollfd* poll_array; // 存放pollfd结构体的数组
    public:
        poll_server(int port = default_port)
            : _port(port), _listensock(-1), poll_array(nullptr)
        {}
        ~poll_server()
        {
            if(_listensock != -1)
                close(_listensock);
            if(poll_array)
                delete[] poll_array;
        }

        void print()
        {
            std::cout << "当前数组中的fd有：";
            for(int i = 0; i < pollfd_size; ++i)
                if(poll_array[i].fd != free_num)
                    std::cout << poll_array[i].fd << " ";
            std::cout << std::endl;
        }

        void Accepter()
        {
            // 1. 获取新链接
            std::string clientip;
            uint16_t clientport;
            int newfd = sock::Accept(_listensock, &clientip, &clientport);
            if(newfd < 0)
                exit(ACCEPT_ERR);
            
            // 2. 将新链接维护到数组中
            // 2.1 首先找到数组中空闲的位置
            int index = 0;
            for(; index < pollfd_size; ++index)
                if(poll_array[index].fd == free_num)
                    break;

            // 2.2 若没有空闲位置，则关闭新链接，并且返回
            if(index == pollfd_size)
            {
                close(newfd);
                logMessage(ERROR, "数组中没有空闲位置，无法建立新链接!"); // 其实也可以搞成动态数组，这里就不弄了
                return;
            }

            // 2.3 找到空闲位置则直接设置进数组即可，顺便打印一下数组的内容
            poll_array[index].fd = newfd;
            poll_array[index].events = POLLIN;
            poll_array[index].revents = 0;
            print();
        }

        void Receiver(int pos)
        {
            // 1. 读取数据
            // 目前我们不做自定义协议，当前我们认为能接收到一个完整的报文
            char buffer[1024];
            memset(buffer, 0, sizeof buffer);
            ssize_t n = recv(poll_array[pos].fd, buffer, sizeof(buffer) - 1, 0);
            if(n > 0)
            {
                buffer[n] = 0;
                logMessage(NORMAL, "接收内容：%s", buffer);
            }
            else if(n == 0)
            {
                // 关闭同时记得将文件描述符从数组中去除
                close(poll_array[pos].fd);
                poll_array[pos].fd = free_num;
                poll_array[pos].events = poll_array[pos].revents = 0;
                logMessage(NORMAL, "客户端关闭连接");
                return;
            }
            else
            {
                // 关闭同时记得将文件描述符从数组中去除
                close(poll_array[pos].fd);
                poll_array[pos].fd = free_num;
                poll_array[pos].events = poll_array[pos].revents = 0;
                logMessage(ERROR, "读写失败，错误码：%d，错误原因：%s", errno, strerror(errno));
                return;
            }

            // 2. 业务处理（这里就不演示了，到后面epoll一起讲）
            // 3. 响应数据，这里直接返回读取的数据
            std::string response = std::string("响应: ") + std::string(buffer);
            send(poll_array[pos].fd, response.c_str(), response.size(), 0);
        }

        void handler()
        {
            // 和select一样，需要遍历处理数组中已经维护文件描述符判断是否有需要处理的就绪事件
            for(int i = 0; i < pollfd_size; ++i)
            {
                // 过滤掉不符合的fd，即不存在、events不是可读事件、revents没就绪，则过滤掉
                if (poll_array[i].fd == free_num || 
                !(poll_array[i].events & POLLIN) || 
                !(poll_array[i].revents == POLLIN))
                    continue;
                
                // 走到这里一定是一个存在且就绪的可读事件！
                if(poll_array[i].fd == _listensock)
                    Accepter();
                else
                    Receiver(i);
            }
        }

        void init()
        {
            _listensock = sock::Socket();
            sock::Bind(_listensock, _port);
            sock::Listen(_listensock);

            // 初始化数组
            poll_array = new struct pollfd[pollfd_size];
            for(int i = 0; i < pollfd_size; ++i)
            {
                poll_array[i].fd = free_num;
                poll_array[i].events = 0; 
                poll_array[i].revents = 0;
            }
            poll_array[0].fd = _listensock;
            poll_array[0].events = POLLIN; // 设置为关心可读事件
        }

        void run()
        {
            while(true)
            {
                int n = poll(poll_array, pollfd_size, 2000); // 2秒超时时间
                if(n == 0)
                    logMessage(Level::NORMAL, "timeout...");
                else if(n == -1)
                    logMessage(Level::ERROR, "poll error, code: %d, err string: %s", errno, strerror(errno));
                else
                {
                    // 说明有事件就绪了
                    logMessage(Level::NORMAL, "有事件就绪了，一共有%d个", n);
                    handler();
                    sleep(1);
                }
            }
        }
    };
}
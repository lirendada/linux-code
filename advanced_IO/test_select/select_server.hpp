#pragma once
#include <iostream>
#include <sys/select.h>
#include "sock.hpp"

namespace select_space
{
    const static int default_port = 8080;           // 服务器默认端口号
    const static int fds_size = sizeof(fd_set) * 8; // 监听集合的文件描述符数量
    const static int free_num = -1;                 // 监听集合中空闲位置的默认值

    class select_server
    {
    private:
        int _listensock;
        int _port;
        int* _array; // 维护已经建立的连接
    public:
        select_server(int port = default_port)
            : _port(port), _listensock(-1), _array(nullptr)
        {}
        ~select_server()
        {
            if(_listensock != -1)
                close(_listensock);
            if(_array)
                delete[] _array;
        }

        void print()
        {
            std::cout << "当前数组中的fd有：";
            for(int i = 0; i < fds_size; ++i)
                if(_array[i] != free_num)
                    std::cout << _array[i] << " ";
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
            for(; index < fds_size; ++index)
                if(_array[index] == free_num)
                    break;

            // 2.2 若没有空闲位置，则关闭新链接，并且返回
            if(index == fds_size)
            {
                close(newfd);
                logMessage(ERROR, "_array中没有空闲位置，无法建立新链接!");
                return;
            }

            // 2.3 找到空闲位置则直接设置进数组即可，顺便打印一下数组的内容
            _array[index] = newfd;
            print();
        }

        void Receiver(int sock, int pos)
        {
            // 1. 读取数据
            // 目前我们不做自定义协议，当前我们认为能接收到一个完整的报文
            char buffer[1024];
            memset(buffer, 0, sizeof buffer);
            ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if(n > 0)
            {
                buffer[n] = 0;
                logMessage(NORMAL, "接收内容：%s", buffer);
            }
            else if(n == 0)
            {
                // 关闭同时记得将文件描述符从数组中去除
                close(sock);
                _array[pos] = free_num;
                logMessage(NORMAL, "文件描述符%d，关闭连接", sock);
                return;
            }
            else
            {
                // 关闭同时记得将文件描述符从数组中去除
                close(sock);
                _array[pos] = free_num;
                logMessage(ERROR, "读写失败，错误码：%d，错误原因：%s", errno, strerror(errno));
                return;
            }

            // 2. 业务处理（这里就不演示了，到后面epoll一起讲）
            // 3. 响应数据，这里直接返回读取的数据
            std::string response = std::string("响应: ") + std::string(buffer);
            send(sock, response.c_str(), response.size(), 0);
        }

        void handler(fd_set& rfds)
        {
            // 需要遍历处理_array数组中已经维护文件描述符判断是否有需要处理的就绪事件
            for(int i = 0; i < fds_size; ++i)
            {
                // 过滤掉不符合的fd
                if (_array[i] == free_num || !FD_ISSET(_array[i], &rfds))
                    continue;
                
                // 走到这里一定是一个存在且就绪的事件！
                if(_array[i] == _listensock)
                    Accepter();
                else
                    Receiver(_array[i], i);
            }
        }

        void init()
        {
            _listensock = sock::Socket();
            sock::Bind(_listensock, _port);
            sock::Listen(_listensock);

            // 初始化数组
            _array = new int[fds_size];
            _array[0] = _listensock;
            for(int i = 1; i < fds_size; ++i)
                _array[i] = free_num;
        }

        void run()
        {
            while(true)
            {
                fd_set rfds; 
                FD_ZERO(&rfds); 

                // 将维护的用来监听的文件描述符，设置到fd_set中，并且寻找描述符中的最大值
                int _maxfd = _array[0];
                for(int i = 0; i < fds_size; ++i)
                {
                    if(_array[i] != free_num)
                    {
                        FD_SET(_array[i], &rfds); 
                        _maxfd = max(_maxfd, _array[i]);
                    }
                }

                int n = select(_maxfd + 1, &rfds, nullptr, nullptr, nullptr); // 注意第一个参数是_maxfd而不是_listensock了!
                if(n == 0)
                    logMessage(Level::NORMAL, "timeout...");
                else if(n == -1)
                    logMessage(Level::ERROR, "select error, code: %d, err string: %s", errno, strerror(errno));
                else
                {
                    // 说明有事件就绪了
                    logMessage(Level::NORMAL, "有事件就绪了，一共有%d个", n);
                    handler(rfds);
                    sleep(1);
                }
            }
        }
    };
}
#pragma once
#include <iostream>
#include <sys/epoll.h>
#include "sock.hpp"

namespace epoll_space
{
    const static int epoll_size = 128;         // 在当前linux版本中，表示epoll_event的默认个数
    const static int defalult_num = 64;        // 就绪数组的默认大小
    const static uint16_t default_port = 8080; // 服务器默认端口号
    const static int buffer_size = 1024;       // 缓冲区大小

    class epoll_server
    {
    public:
        epoll_server(uint16_t port = default_port, int num = defalult_num)
            : _port(port), _num(num)
        {
            // 1. 完成套接字基本流程
            _listensock = sock::Socket();
            sock::Bind(_listensock, _port);
            sock::Listen(_listensock);

            // 2. 创建epoll模型
            _epfd = epoll_create(epoll_size);
            if(_epfd == -1)
            {
                logMessage(Level::FATAL, "create epoll error");
                exit(EPOLL_CREATE_ERR);
            }
            logMessage(Level::NORMAL, "create epoll success");

            // 3. 添加_listensock到epoll中，并且关心可读事件
            struct epoll_event in;
            in.data.fd = _listensock;
            in.events = EPOLLIN;
            int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, _listensock, &in);
            if(ret == -1)
            {
                logMessage(Level::ERROR, "add _listensock error");
                exit(EPOLL_CTL_ERR);
            }
            logMessage(Level::NORMAL, "add _listensock success");

            // 4. 开辟就绪事件数组的空间
            _events = new struct epoll_event[_num];
            if(_events == nullptr)
            {
                logMessage(Level::ERROR, "new epoll_events error");
                exit(NEW_EVENTS_ERR);
            }
            logMessage(Level::NORMAL, "init server success");
        }

        ~epoll_server()
        {
            if(_listensock)
                close(_listensock);
            if(_epfd)
                close(_epfd);
            if(_events)
                delete[] _events;
        }

        void run()
        {
            while(true)
            {
                // 5. 进行就绪事件的查询等待
                int n = epoll_wait(_epfd, _events, _num, 2000);
                if(n == -1)
                    logMessage(Level::ERROR, "epoll_wait error, code: %d, strerror: %s", errno, strerror(errno));
                else if(n == 0)
                    logMessage(Level::NORMAL, "timeout...");
                else
                {
                    logMessage(Level::NORMAL, "一共有%d个事件就绪", n);
                    handler(n);
                }
            }
        }

        void handler(int ready_num)
        {
            char buffer[1024];
            for(int i = 0; i < ready_num; ++i)
            {
                
            }
        }
    private:
        uint16_t _port;
        int _listensock;

        int _epfd;                   // epoll模型的文件描述符
        int _num;                    // 就绪事件数组的大小
        struct epoll_event* _events; // 就绪事件数组
    };
}
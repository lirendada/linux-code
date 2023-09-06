#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include "sock.hpp"

namespace epoll_space
{
    const static int epoll_size = 128;         // 在当前linux版本中，表示epoll_event的默认个数
    const static int defalult_num = 64;        // 就绪数组的默认大小
    const static uint16_t default_port = 8080; // 服务器默认端口号
    const static int buffer_size = 1024;       // 缓冲区大小

    class epoll_server
    {
    private:
        uint16_t _port;
        int _listensock;
        int _epfd;                   // epoll模型的文件描述符
        int _num;                    // 就绪事件数组的大小
        struct epoll_event* _events; // 就绪事件数组

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

            // 3. 添加_listensock到epoll中，并且关心可读事件和启动ET模式
            AddFD(_listensock, true);

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
                sleep(1);
            }
        }

        void handler(int ready_num)
        {
            for(int i = 0; i < ready_num; ++i)
            {
                // 这里遍历的事件都是就绪的！
                // 1. 首先获取事件类型以及文件描述符
                uint32_t event = _events[i].events;
                int fd = _events[i].data.fd;

                // 2. 根据不同的事件类型以及文件描述符来做不同的业务处理
                if(fd == _listensock && (event & EPOLLIN))
                {
                    // 如果是_listensock并且是可读事件，则获取新链接
                    Accepter();
                }
                else if(event & EPOLLIN)
                {
                    // 如果是普通文件描述符的可读事件，我们就接收信息
                    Receiver(fd);
                }
                else
                {
                    // 如果是普通文件描述符的可写事件等等，我们这里不做处理，等后面将reactor的时候一起写
                }
            }
        }
    private:
        // 设置文件描述符为阻塞模式
        void SetNonBlocking(int fd)
        {
            // 1. 先获取文件描述符标记
            int old_option = fcntl(fd, F_GETFL);

            // 2. 设为非阻塞模式
            int new_option =old_option | O_NONBLOCK;
            fcntl(fd, new_option);
        }
        
        // 添加文件描述符到epoll模型中，如果有必要的话还可以设置为ET模式
        void AddFD(int fd, bool enable_ET)
        {
            // 1. 设置为可读事件
            struct epoll_event in;
            in.data.fd = fd;
            in.events = EPOLLIN;

            // 2. 看看是否需要设置为ET模式
            if(enable_ET)
                in.events |= EPOLLET;
            
            // 3. 添加到epoll模型中
            int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &in);
            if(ret == -1)
            {
                logMessage(Level::ERROR, "epoll_ctl error, fd: %d, errno: %d, why: %s", fd, errno, strerror(errno));
                exit(EPOLL_CTL_ERR);
            }
            logMessage(Level::NORMAL, "epoll_ctl success, fd: %d", fd);

            // 4. 设置为非阻塞模式
            SetNonBlocking(fd);
        }

        void Accepter()
        {
            // 1. 获取新连接
            std::string clientip;
            uint16_t clientport;
            int newfd = sock::Accept(_listensock, &clientip, &clientport);

            // 2. 将该新连接交给epoll模型管理
            AddFD(newfd, true);;
        }

        void Receiver(int fd)
        {
            char buffer[1024];
            ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0);

            if(n == -1)
            {
                // 读取发生错误，将该事件从epoll模型中去除，然后关闭
                epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                logMessage(Level::ERROR, "receive error, fd: %d, errno: %d, why: %s", fd, errno, strerror(errno));
            }
            else if(n == 0)
            {
                // 读到0表示请求断开连接，也是一样将该事件从epoll模型中去除，然后关闭
                epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                logMessage(Level::NORMAL, "receive close, fd: %d", fd);
            }
            else
            {
                buffer[n] = 0;
                logMessage(Level::NORMAL, "receive success, fd: %d, 内容: %s", fd, buffer);

                // 这里做简单的回响处理
                std::string response = "响应: " + std::string(buffer);
                send(fd, response.c_str(), response.size(), 0);
            }
        }
    };
}
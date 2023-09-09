#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include "sock.hpp"
#include "err.hpp"
#include "log.hpp"

const static int time_out = 1000;  // epoll_wait的超时时间

class epoller
{
private:
    int _epfd; // epoll模型的文件描述符
public:
    epoller()
        : _epfd(default_num)
    {}

    // 创建epoll模型，并且开辟就绪事件数组
    int create(int epoll_size)
    {
        // 创建epoll模型
        _epfd = epoll_create(epoll_size);
        if(_epfd == -1)
        {
            logMessage(Level::ERROR, "epoll_create error, errno: %d, string_err: %s", errno, strerror(errno));
            exit(EPOLL_CREATE_ERR);
        }
        logMessage(Level::NORMAL, "epoll_create success");
        return _epfd;
    }

    // 操作epoll模型
    void control(int fd, int option, uint32_t events)
    {
        if(option == EPOLL_CTL_ADD | option == EPOLL_CTL_MOD)
        {
            struct epoll_event ev;
            ev.data.fd = fd;
            ev.events = events;

            // 设置非阻塞
            setnonblocking(fd);

            int ret = epoll_ctl(_epfd, option, fd, &ev);
            if(ret == -1)
            {
                logMessage(Level::ERROR, "epoll_add or mod error, errno: %d, string_err: %s", errno, strerror(errno));
                exit(EPOLL_CTL_ERR);
            }
            logMessage(Level::NORMAL, "epoll_add or mod success");
        }
        else if(option == EPOLL_CTL_DEL)
        {
            int ret = epoll_ctl(_epfd, option, fd, nullptr);
            if(ret == -1)
            {
                logMessage(Level::ERROR, "epoll_del error, errno: %d, string_err: %s", errno, strerror(errno));
                exit(EPOLL_CTL_ERR);
            }
            logMessage(Level::NORMAL, "epoll_del success");
        }
    }

    // 等待就绪事件，并且进行任务的分配
    int wait(struct epoll_event* events, int maxevents)
    {
        int n = epoll_wait(_epfd, events, maxevents, time_out);
        return n;
    }
private:
    // 设置非阻塞状态
    void setnonblocking(int fd)
    {
        // 获取原先状态
        int old_option = fcntl(fd, F_GETFL);
        if(old_option == -1)
        {
            logMessage(Level::ERROR, "get non_blocking error, errno: %d, string_err: %s", errno, strerror(errno));
            exit(GET_NON_BLOCKING_ERR);
        }

        // 设置非阻塞状态
        int new_option = fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
        if(new_option == -1)
        {
            logMessage(Level::ERROR, "set non_blocking error, errno: %d, string_err: %s", errno, strerror(errno));
            exit(SET_NON_BLOCKING_ERR);
        }
    }
};
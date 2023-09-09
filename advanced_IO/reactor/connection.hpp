#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <unistd.h>
#include "reactor.hpp"

class reactor;
class connection;
using func_t = std::function<void(connection* conn)>;

class connection
{
public:
    int _fd;
    std::string _inbuffer;  // 输入缓冲区
    std::string _outbuffer; // 输出缓冲区

    func_t _receiver;  // 可读事件处理函数
    func_t _sender;    // 可写事件处理函数
    func_t _excepter; // 异常事件处理函数

    reactor* _rp; // 指向reactor的指针，是为了方便找到_rp
public:
    connection(int fd, reactor* rp, func_t receiver, func_t sender, func_t excepter)
        : _fd(fd), _rp(rp), _receiver(receiver), _sender(sender), _excepter(excepter)
    {}
    ~connection()
    {
        Close();
    }
    void Close()
    {
        if(_fd != -1)
        {
            close(_fd);
            _fd = -1;
        }
    }
};
#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "err.hpp"
#include "log.hpp"

const static int maxbacklog = 32;
const static int default_num = -1;

class sock
{
private:
    int _listensock = default_num; 
public:
    sock() {}
    ~sock() { Close(); }

    int GetFD() { return _listensock; }

    void Socket()
    {
        // 1. 创建套接字
        _listensock = socket(AF_INET, SOCK_STREAM, 0);
        if(_listensock < 0)
        {
            logMessage(Level::ERROR, "socket error: %s", strerror(errno));
            exit(SOCKET_ERR);
        }
        logMessage(Level::NORMAL, "create socket success: %d", _listensock);

        // 1.1 设置地址复用
        int opt = 1;
        setsockopt(_listensock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    }

    void Bind(int port)
    {
        // 2. 绑定套接字信息
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(_listensock, (struct sockaddr*)&local, sizeof(local)) < 0)
        {
            logMessage(Level::ERROR, "bind error: %s", strerror(errno));
            exit(BIND_ERR);
        }
        logMessage(Level::NORMAL, "bind succuess");
    }

    void Listen()
    {
        // 3. 设置socket为监听状态
        if(listen(_listensock, maxbacklog) < 0)
        {
            logMessage(Level::ERROR, "listen error: %s", strerror(errno));
            exit(LISTEN_ERR);
        }
        logMessage(Level::NORMAL, "listen succuess");
    }

    int Accept(std::string* clientip, uint16_t* clientport, int* err)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int fd = accept(_listensock, (struct sockaddr*)&client, &len);
        *err = errno;
        if(fd >= 0)
        {
            *clientip = inet_ntoa(client.sin_addr);
            *clientport = ntohs(client.sin_port);
        }
        return fd;
    }
private:
    void Close()
    {
        if(_listensock != default_num)
        {
            close(_listensock);
            _listensock = default_num;
        }
    }
};
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

class sock
{
    const static int maxbacklog = 32;
public:
    static int Socket()
    {
        // 创建套接字
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd < 0)
        {
            logMessage(Level::ERROR, "socket error: %s", strerror(errno));
            exit(SOCKET_ERR);
        }
        logMessage(Level::NORMAL, "create socket success: %d", fd);

        // 设置地址复用
        int opt = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
        return fd;
    }

    static void Bind(int fd, int port)
    {
        // 绑定套接字信息
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        if(bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0)
        {
            logMessage(Level::ERROR, "bind error: %s", strerror(errno));
            exit(BIND_ERR);
        }
        logMessage(Level::NORMAL, "bind succuess");
    }

    static void Listen(int fd)
    {
        if(listen(fd, maxbacklog) < 0)
        {
            logMessage(Level::ERROR, "listen error: %s", strerror(errno));
            exit(LISTEN_ERR);
        }
        logMessage(Level::NORMAL, "listen succuess");
    }

    static int Accept(int listenfd, std::string* clientip, uint16_t* clientport)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int fd = accept(listenfd, (struct sockaddr*)&client, &len);
        if(fd < 0)
        {
            logMessage(Level::ERROR, "accept error: %s", strerror(errno));
        }
        else
        {
            logMessage(Level::NORMAL, "accept succuess, the fd is %d", fd);
            *clientip = inet_ntoa(client.sin_addr);
            *clientport = ntohs(client.sin_port);
        }
        return fd;
    }
};
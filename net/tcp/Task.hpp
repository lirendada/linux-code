#pragma once 
#include <iostream>
#include <functional>
#include <cstdio>
#include <string>
#include <fstream>
#include "tcpserver.hpp"
#include "log.hpp"

void service_IO(int sockfd)
{
    char buffer[1024];
    while(true)
    {
        ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
        if(n == -1)
        {
            logMessage(Level::ERROR, "read error");
            exit(1);
        }
        else if(n == 0) // 代表客户端退出
        {
            logMessage(Level::NORMAL, "client quit and I must quit, too!");
            break;
        }
        else
        {
            buffer[n] = '\0';
            cout << "receive message is: " << buffer << endl;

            // 写回给客户端
            string outbuffer = buffer;
            outbuffer += "server[echo]";
            write(sockfd, outbuffer.c_str(), outbuffer.size());
        }
    }
    close(sockfd);
}  

class Task
{
    using func_t = std::function<void(int)>;
public:
    Task()
    {}
    Task(int sockfd, func_t func)
        :_sockfd(sockfd), _callback(func)
    {}
    void operator()()
    {
        _callback(_sockfd);
    }
private:
    int _sockfd;
    func_t _callback;
};
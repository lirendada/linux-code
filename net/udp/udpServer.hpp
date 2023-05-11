#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "onlineUser.hpp"
using namespace std;

namespace Server
{
    static const string defaultIP = "0.0.0.0"; // 默认设为0，表示接收任意IP
    static const int MAXSIZE = 1024; // 接收到的数据最大值

    enum { USAGE_ERR = 1, BIND_ERR, SOCKET_ERR, CLOSE_ERR, OPEN_ERR, NOTFOUND_ERR, SEND_ERR };

    using func_t = function<void(int, string, uint16_t, string)>;

    class udpServer
    {
    public:
        udpServer(func_t callback, const uint16_t port, const string& ip = defaultIP)
            :_port(port), _ip(ip), _socketfd(-1), _callback(callback)
        {}

        // 初始化服务端
        void initServer()
        {
            // 1.创建套接字（本质是创建文件）
            _socketfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(_socketfd == -1)
            {
                cerr << "socket error: " << errno << " : " << strerror(errno) << endl; 
                exit(SOCKET_ERR);
            }
            cout << "socket success: " << _socketfd << endl;

            // 2.绑定端口号和ip地址到当前的套接字文件
            struct sockaddr_in local;
            bzero(&local, sizeof local); // 先将一段local的内存清零，即将其中的每个字节都设置为0
            
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);  // 因为端口号是多个字节组成，所以要保证先转化为大段序列

            // inet_addr函数帮我们将格式化字符串转化为in_addr_t类型，并且调整成大段序列
            // local.sin_addr.s_addr = inet_addr(_ip.c_str()); 

            // 但是一般我们将作为服务器的ip设为全0，所以不需要做上述工作，直接利用一个值为全0的宏赋值就行
            local.sin_addr.s_addr = INADDR_ANY;

            int n = bind(_socketfd, (struct sockaddr*)&local, sizeof local);
            if(n == -1)
            {
                cerr << "bind error: " << errno << " : " << strerror(errno) << endl; 
                exit(BIND_ERR);
            }
        }

        // 启动服务端
        void start()
        {
            // 服务器的本质就是一个死循环，称为常驻内存的进程
            char buffer[MAXSIZE];
            while(true)
            {
                struct sockaddr_in src;
                socklen_t srclen = sizeof(src); // 这是因为操作系统不知道我们传过去的是哪个sockaddr的哪个结构体，所以我们要传大小过去

                // 这里要传sizeof(buffer)-1是因为腾出一个位置给\0
                ssize_t n = recvfrom(_socketfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&src, &srclen);
                if(n > 0)
                {
                    // 将接收到的客户端的信息保存起来
                    uint16_t src_port = ntohs(src.sin_port); // 要考虑大端接收问题，所以要转化一下
                    string src_ip = inet_ntoa(src.sin_addr); // 考虑到原本是个uint32_t类型，且还要转化为点分法，所以我们借用inet_ntoa函数帮我们完成
                    
                    buffer[n] = '\0';
                    string recvmessage = buffer;

                    // 打印收集到的信息，并且执行任务
                    cout << "[" << src_ip << ", " << src_port << "]: " <<  recvmessage << endl;
                    _callback(_socketfd, src_ip, src_port, recvmessage);
                }
            }
        }

        ~udpServer()
        {
            int n = close(_socketfd);
            if(n != 0)
            {
                cout << "close error: " << errno << " : " << strerror(errno) << endl;
                exit(CLOSE_ERR); 
            }
        }
    private:
        uint16_t _port; // 当前服务端进程的端口号
        string _ip;     // 当前服务端的ip，但是作为一个服务器，一般都是将ip设为全0，代表任意ip都能访问
        int _socketfd;  // 套接字文件的文件描述符
        func_t _callback; // 服务端要完成的业务
    };
}
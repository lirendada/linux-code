#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include "log.hpp"
#include "protocol.hpp"
using namespace std;

namespace Server
{
    enum {
        SOCKET_ERR = 1,
        BIND_ERR,
        LISTEN_ERR,
        ACCEPT_ERR,
        READ_ERR,
        WRITE_ERR,
    };

    const int gbacklog = 5;
    const uint16_t gport = 8080;

    // req：输入型参数
    // resp：输出型参数
    typedef function<bool(const Request& req, Response& resp)> func_t;

    // 保证解耦
    void handlerEntery(int sockfd, func_t func)
    {
        string recvbuffer;
        while(true)
        {
            // 1. 读取客户端发来的数据
            // 1.1 你怎么保证你读到的消息是【一个】完整的请求？？？由recvPackage帮我们完成
            string recv_string;
            if(!recvPackage(sockfd, recvbuffer, &recv_string))
                return;
            // 1.2 要将收到的请求进行去掉自定义规则部分
            string request_string;
            if(!delRule(recv_string, &request_string))
                return;

            // 2. 对请求request进行反序列化
            // 2.1 得到一个结构化的请求对象
            Request req;
            if(!req.deserialize(request_string))
                return;

            // 3. 计算处理，通过对象获取对应的数据进行计算 --- 业务逻辑
            // 3.1 得到一个结构化的响应
            Response resp;
            func(req, resp);

            // 4. 对响应response进行序列化
            // 4.1 得到一个序列化的“字符串”
            string response_string;
            if(!resp.serialize(&response_string))
                return;

            // 5. 然后再发送响应
            // 5.1 发送之前先加上自定义规则
            string send_string = addRule(response_string);
            // 5.2 再将包装好的数据发送出去
            send(sockfd, send_string.c_str(), send_string.size(), 0); // 这里有问题，后面再说
        }
    }

    class tcpServer
    {
    public:
        tcpServer(const uint16_t& port = gport) : _port(port), _listenfd(0)
        {}

        void initServer()
        {
            // 1.创建套接字
            _listenfd = socket(AF_INET, SOCK_STREAM, 0);
            if(_listenfd == -1)
            {
                logMessage(Level::FATAL, "socket error");
                exit(SOCKET_ERR);
            }
            logMessage(Level::NORMAL, "socket success: %d", _listenfd);

            // 2.绑定信息
            struct sockaddr_in local;
            memset(&local, 0, sizeof local);
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = INADDR_ANY;

            if(bind(_listenfd, (struct sockaddr*)&local, sizeof local) == -1)
            {
                logMessage(Level::FATAL, "bind error");
                exit(BIND_ERR);
            }
            logMessage(Level::NORMAL, "bind success");

            // 3.设置listen监听状态
            if(listen(_listenfd, gbacklog) < 0)
            {
                logMessage(Level::FATAL, "listen error");
                exit(LISTEN_ERR);
            }
            logMessage(Level::NORMAL, "listen success");
        }

        void start(func_t func)
        {
            signal(SIGCHLD, SIG_IGN);
            while(true)
            {
                // 4.若监听到客户端的信息之后，进行accept
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int server_sockfd = accept(_listenfd, (struct sockaddr*)&peer, &len);
                if(server_sockfd == -1)
                {
                    logMessage(Level::ERROR, "accept error");
                    continue;
                }
                logMessage(Level::NORMAL, "accept success, get new sockfd: %d", server_sockfd);
                cout << "server_sockfd: " << server_sockfd << endl;

                // version2 多进程版本，主要学习思想，借助的是孙子进程或者不被父进程负责回收的子进程
                pid_t id= fork();
                if(id == 0)
                {
                    close(_listenfd);
                    handlerEntery(server_sockfd, func); // 执行我们的服务器处理函数
                    close(server_sockfd);
                    exit(0);
                }
                close(server_sockfd); 
                pid_t ret = waitpid(id, nullptr, 0); 
                if(ret > 0)
                    cout << "wait success, this child id is " << ret << endl;
            }
        }

        ~tcpServer()
        {}
    private:
        uint16_t _port;
        int _listenfd;
    };
}
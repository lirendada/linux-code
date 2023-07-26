#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "protocol.hpp"
using namespace std;

const uint16_t gport = 8080;
const int gbacklog = 5;
using func_t = function<bool(int, const httpRequest&, httpResponse&)>;

class ThreadData
{
public:
    ThreadData(int sockfd, func_t func)
        : _socketfd(sockfd), _func(func)
    {}
public:
    func_t _func;
    int _socketfd;
};

void handlerHttp(ThreadData* td)
{
    // 1. 读取http请求
    // 2. 反序列化（这里省略）
    // 3. 业务逻辑
    // 4. 序列化（这里省略）
    // 5. 发送http响应回去
    char buffer[4096];
    httpRequest req;
    httpResponse resp;

    ssize_t n = recv(td->_socketfd, buffer, sizeof(buffer)-1, 0); // 大概率会收到一个完整的请求
    if(n > 0)
    {
        buffer[n] = 0;
        req.inbuffer = buffer;
        req.parse();
        td->_func(td->_socketfd, req, resp);
        send(td->_socketfd, resp.outbuffer.c_str(), resp.outbuffer.size(), 0);
    }
}

class httpserver
{
public:
    httpserver(func_t func, const uint16_t& port = gport) 
        : _func(func), _port(port), _listenfd(-1)
    {}
    void start()
    {
        // 创建套接字
        _listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if(_listenfd == -1)
        {
            cout << "socket error" << endl;
            exit(1);
        }
        cout << "socket success" << endl;

        // 绑定网络信息
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY;
        int n = bind(_listenfd, (struct sockaddr*)&local, sizeof(local));
        if(n == -1)
        {
            cout << "bind error" << endl;
            exit(1);
        }
        cout << "bind success" << endl;

        // 监听
        n = listen(_listenfd, gbacklog);
        if(n == -1)
        {
            cout << "listen error" << endl;
            exit(1);
        }
        cout << "listen success" << endl;
    }
    void run()
    {
        while(true)
        {
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int serverfd = accept(_listenfd, (struct sockaddr*)&client, &len);
            if(serverfd == -1)
            {
                cout << "accept error" << endl;
                exit(1);
            }
            cout << "accept success" << endl;

            pthread_t tid;
            ThreadData* td = new ThreadData(serverfd, _func);
            pthread_create(&tid, nullptr, thread_routine, (void*)td);
        }
    }
    static void* thread_routine(void* args)
    {
        pthread_detach(pthread_self());
        ThreadData* td = static_cast<ThreadData*>(args);
        handlerHttp(td);
        close(td->_socketfd);
        return nullptr;
    }
private:
    func_t _func;
    uint16_t _port;
    int _listenfd;
};
#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include "ThreadPool.hpp"
#include "Task.hpp"
#include "log.hpp"
using namespace std;

namespace Server
{
    enum {
        SOCKET_ERR = 1,
        BIND_ERR,
        LISTEN_ERR,
        ACCEPT_ERR,
        READ_ERR,
        WRITE_ERR
    };

    const int gbacklog = 5;
    const uint16_t gport = 8080;

    class tcpServer;
    class ThreadData
    {
    public:
        tcpServer* _self;
        int _sockfd;
        ThreadData(tcpServer* self, int sockfd)
            :_self(self), _sockfd(sockfd)
        {}
    };

    class tcpServer
    {
    public:
        tcpServer(const uint16_t& port = gport) : _port(port), _listenfd(-1)
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

            if((bind(_listenfd, (struct sockaddr*)&local, sizeof local)) < 0)
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

        void start()
        {
            // 初始化线程池
            ThreadPool<Task>::GetInstance()->run();
            logMessage(Level::NORMAL, "Thread init success");

            // signal(SIGCHLD, SIG_IGN);
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

                // version1，客户端之间的通信都是阻塞的
                // service_IO(server_sockfd);
                // close(server_sockfd); // 对使用完的套接字需要关闭，不然导致描述符泄漏问题

                // version2 多进程版本，主要学习思想，借助的是孙子进程或者不被父进程负责回收的子进程
                // pid_t id= fork();
                // if(id == 0)
                // {
                //     // children
                //     close(_listenfd); // 关闭子进程拷贝的父进程中不必要的描述符比如这里的监听，以免误操作
                //     // if(fork() > 0) // 让当前的子进程退出
                //     //     exit(1); 
                //     // 也就是说到这里就剩下一个孙子进程，而它变成了孤儿进程，由OS来管理回收
                //     service_IO(server_sockfd); 
                //     close(server_sockfd);
                //     exit(0);
                // }
                // father，如果不是用信号忽略子进程的捕捉的话，那么就要等待
                // 并且要阻塞等待，因为非阻塞等待的话如果描述符用完了，但是轮询时候访问到上面的accept
                // accept的返回值就是错误的了，那么server_sockfd就失效了，也就是回收不了进程了
                // close(server_sockfd); 
                // pid_t ret = waitpid(id, nullptr, 0); 
                // if(ret > 0)
                //     cout << "wait success, this child id is " << ret << endl;

                // version3 多线程版本，不能线程等待，
                // 因为这样子就变成了线程阻塞的形式了，最好就是用线程分离
                // pthread_t tid;
                // ThreadData* td = new ThreadData(this, server_sockfd);
                // pthread_create(&tid, nullptr, thread_routine, td);

                // version4 线程池版本，套用我们以前写的线程池
                // 并且这是一个单例对象，初始化在上面，这里是调用put了
                ThreadPool<Task>::GetInstance()->put(Task(server_sockfd, service_IO));
            }
        }

        // static void* thread_routine(void* args)
        // {
        //     // 记得线程分离
        //     pthread_detach(pthread_self());

        //     ThreadData* td = static_cast<ThreadData*>(args);
        //     td->_self->service_IO(td->_sockfd);
        //     close(td->_sockfd);
        //     delete td;
        //     return nullptr;
        // }

        ~tcpServer()
        {}
        
        // 将该函数放到Task.hpp中
        // void service_IO(int sockfd)
        // {
        //     char buffer[1024];
        //     while(true)
        //     {
        //         ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
        //         if(n == -1)
        //         {
        //             logMessage(Level::ERROR, "read error");
        //             exit(READ_ERR);
        //         }
        //         else if(n == 0) // 代表客户端退出
        //         {
        //             logMessage(Level::NORMAL, "client quit and I must quit, too!");
        //             break;
        //         }
        //         else
        //         {
        //             buffer[n] = '\0';
        //             cout << "receive message is: " << buffer << endl;

        //             // 写回给客户端
        //             string outbuffer = buffer;
        //             outbuffer += "server[echo]";
        //             write(sockfd, outbuffer.c_str(), outbuffer.size());
        //         }
        //     }
        // }  
    private:
        string _ip;
        uint16_t _port;
        int _listenfd;
    };
}
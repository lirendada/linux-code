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
#include "protocol.hpp"
#include "log.hpp"
using namespace std;

namespace Client
{
    const int NUM = 1024;
    
    // 类外实现，保证与服务器解耦
    void handler(int sockfd)
    {
        string msg;      // 要发送的消息
        string inbuffer; // 接收缓冲区
        while(true)
        {
            cout << "Enter>>> ";
            getline(cin, msg); // 输入的消息形式如"1+1"、"212345*131"
            
            // 1. 首先客户端肯定是创建请求并且发送给服务端，发送之前要先序列化，
            Request req;
            if(!get_req_from_string(msg, req))
            {
                logMessage(Level::ERROR, "Your expression format input error!"); // 输入格式错误
                continue;
            }
            string send_body;
            if(!req.serialize(&send_body)) // 序列化
            {
                logMessage(Level::ERROR, "serialize error!");
                continue;
            }

            // 2. 然后加上自定义协议再发送
            string send_string = addRule(send_body); 
            send(sockfd, send_string.c_str(), send_string.size(), 0); // 这里有问题，后面再说

            // 3. 接收来自服务端的响应，和服务端一样，使用协议头文件中的recvPackage函数来替我们完成即可
            string recv_string;
            if(!recvPackage(sockfd, inbuffer, &recv_string))
                continue;   // 没有读到完整的报文则继续读

            // 4. 读到响应之后进行去除协议
            string recv_body;
            if(!delRule(recv_string, &recv_body)) // 去除协议规则
            {
                logMessage(Level::ERROR, "delRule error!");
                continue;
            }

            // 5. 并且进行反序列化
            Response resp;
            if(!resp.deserialize(recv_body)) // 反序列化
            {
                logMessage(Level::ERROR, "deserialize error!");
                continue;
            }

            // 6. 打印响应报文的数据
            cout << "exitcode: " << resp._exitcode << endl;
            cout << "result: " << resp._result << endl;
        }
    }

    class tcpClient
    {
    private:
        int _socketfd;
        string _destip;
        uint16_t _destport;

    public:
        tcpClient(const string& ip, const uint16_t& port)
            :_destip(ip), _destport(port), _socketfd(0)
        {}

        void initClient()
        {
            // 1.创建套接字
            _socketfd = socket(AF_INET, SOCK_STREAM, 0);
            if(_socketfd < 0)
            {
                std::cerr << "socket create error" << std::endl;
                exit(2);
            }
            // 2. tcp客户端不需要手动bind
        }

        void run()
        {
            struct sockaddr_in server;
            memset(&server, 0, sizeof server);
            server.sin_family = AF_INET;
            server.sin_port = htons(_destport);
            server.sin_addr.s_addr = inet_addr(_destip.c_str());

            // 使用connect函数发送建立连接请求
            int n = connect(_socketfd, (struct sockaddr*)&server, sizeof server);
            if(n == -1)
                logMessage(Level::ERROR, "socket connect error!");
            else
                handler(_socketfd); // 直接调用处理函数
        }

        ~tcpClient()
        {
            if(_socketfd >= 0) 
                close(_socketfd);
        }
    };
}
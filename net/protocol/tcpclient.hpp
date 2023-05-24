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
using namespace std;

namespace Client
{
    const int NUM = 1024;

    class tcpClient
    {
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
            // 2. tcp的客户端要不要bind？要的！ 要不要显示的bind？不要！这里尤其是client port要让OS自定随机指定！
            // 3. 要不要listen？不用！
            // 4. 要不要accept? 不用！
            // 5. 要什么呢？？要发起连接！
        }

        void run()
        {
            struct sockaddr_in server;
            memset(&server, 0, sizeof server);
            server.sin_family = AF_INET;
            server.sin_port = htons(_destport);
            server.sin_addr.s_addr = inet_addr(_destip.c_str());

            // 连接使用的是connect函数
            int n = connect(_socketfd, (struct sockaddr*)&server, sizeof server);
            if(n == -1)
            {
                cerr << "socket connect error" << std::endl;
            }
            else
            {
                string msg;
                string inbuffer;
                while(true)
                {
                    cout << "Enter>>> ";
                    getline(cin, msg);

                    // 1. 首先肯定是创建请求并且发送给服务端
                    // 1.1 发送之前要先序列化，然后加上自定义规则再发送
                    Request req(10, 10, '+');
                    string send_body;
                    if(!req.serialize(&send_body)) // 序列化
                        continue;
                    string send_string = addRule(send_body); // 添加自定义协议
                    send(_socketfd, send_string.c_str(), send_string.size(), 0); // 发送，有bug，后面再说

                    // 2. 接收来自服务端的响应，和服务端一样，使用recvPackage来替我们完成即可
                    string recv_string;
                    if(!recvPackage(_socketfd, inbuffer, &recv_string))
                        continue; // 没有读到完整的报文则继续读

                    // 读到之后进行去除协议和反序列化
                    string recv_body;
                    if(!delRule(recv_string, &recv_body)) // 去除协议规则
                        continue;
                    Response resp;
                    if(!resp.deserialize(recv_body)) // 反序列化
                        continue;

                    // 3. 打印响应报文的数据
                    cout << "exitcode: " << resp._exitcode << endl;
                    cout << "result: " << resp._result << endl;
                }
            }
        }

        ~tcpClient()
        {
            if(_socketfd >= 0) 
                close(_socketfd);
        }
    private:
        int _socketfd;
        string _destip;
        uint16_t _destport;
    };
}
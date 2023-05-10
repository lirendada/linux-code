#pragma once
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

namespace Client
{
    enum { USAGE_ERR = 1, SEND_ERR, SOCKET_ERR, CLOSE_ERR };

    class udpClient
    {
    public:
        udpClient(const string& ip, const uint16_t port)
            : _serverPort(port), _serverIP(ip), _socketfd(-1), _quit(false)
        {}

        void initClient()
        {
            // 1.创建套接字文件
            _socketfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(_socketfd == -1)
            {
                cerr << "socket error: " << errno << " : " << strerror(errno) << endl; 
                exit(SOCKET_ERR);
            }
            cout << "socket success: " << _socketfd << endl;

            // 2.绑定当前客户端的ip和端口号到套接字，而ip和端口号其实不需要明确绑定，
            // 因为套接字会自动绑定到系统分配的本地IP地址和端口号上，所以一般我们可以不显式绑定！       
        }

        void run()
        {   
            // 将要发送到目的端的信息填上
            struct sockaddr_in destination;
            memset(&destination, 0, sizeof destination);
            destination.sin_family = AF_INET;
            destination.sin_addr.s_addr = inet_addr(_serverIP.c_str());
            destination.sin_port = htons(_serverPort);

            string message; // 要发送的信息
            while(!_quit)
            {
                // 发送信息，建议还是统一使用C语言的形式使用读写操作
                cout << "Please enter the message you want to send: ";
                char line[1024];
                fgets(line, sizeof(line), stdin);
                message = line;

                ssize_t n = sendto(_socketfd, message.c_str(), message.size(), 0, (struct sockaddr*)&destination, sizeof(destination));
                if(n == -1)
                {
                    cerr << "send error: " << errno << " : " << strerror(errno) << endl; 
                    exit(SEND_ERR);
                }

                // 接收信息，但是后面这里我们会改成多线程，因为上面的输入导致了阻塞
                char buffer[1024];
                struct sockaddr_in tmp;
                socklen_t tmplen = sizeof(tmp);
                ssize_t s = recvfrom(_socketfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&tmp, &tmplen);
                if(s > 0)
                    buffer[s] = '\0';
                cout << "服务器的翻译结果#\n" << buffer << endl;
            }
        }

        ~udpClient()
        {}
    private:
        uint16_t _serverPort;
        string _serverIP;
        int _socketfd; 
        bool _quit;
    };
}
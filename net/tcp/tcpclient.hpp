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
using namespace std;

namespace Client
{
    const int NUM = 1024;

    class tcpClient
    {
    public:
        tcpClient(const string& ip, const uint16_t& port)
            :_destip(ip), _destport(port), _socketfd(-1)
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
            // 3. 要不要listen？不要！
            // 4. 要不要accept? 不要！
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
                std::string msg;
                while(true)
                {
                    cout << "Enter# ";
                    getline(cin, msg);
                    send(_socketfd, msg.c_str(), msg.size(), 0);

                    char buffer[NUM];
                    int n = recv(_socketfd, buffer, sizeof(buffer)-1, 0);
                    if(n > 0)
                    {
                        // 目前我们把读到的数据当成字符串, 截止目前
                        buffer[n] = 0;
                        cout << "Server回显# " << buffer << endl;
                    }
                    else
                    {
                        break;
                    }
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
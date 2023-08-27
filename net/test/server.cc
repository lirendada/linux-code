#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace std;

const uint16_t port = 8080;

int main()
{
    // 创建套接字
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1)
    {
        cout << "socket error" << endl;
        exit(1);
    }
    
    // 设置地址复用
    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定套接字信息
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(socketfd, (struct sockaddr*)&local, sizeof(local)) == -1)
    {
        cout << "bind error" << endl;
        exit(2);
    }

    // 监听套接字
    // 并将backlog参数设为1
    if(listen(socketfd, 1) == -1) 
    {
        cout << "listen error" << endl;
        exit(3);
    }

    // 死循环，不调用accept获取连接
    while(1)
    {}
    
    return 0;
}
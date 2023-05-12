#include "udpServer.hpp"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
#include <stdio.h>
using namespace std;
using namespace Server;
static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}
void handlerMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    string response_message;
    response_message += " [server echo]";

    // 发回给客户端
    struct sockaddr_in client;
    bzero(&client, 0);
    client.sin_family = AF_INET;
    client.sin_port = htons(clientport);
    client.sin_addr.s_addr = inet_addr(clientip.c_str());

    ssize_t n = sendto(sockfd, response_message.c_str(), response_message.size(), 0, (struct sockaddr*)&client, sizeof client);
    if(n == -1)
    {
        cerr << "send error: " << errno << " : " << strerror(errno) << endl; 
        exit(SEND_ERR);
    }
}
int main(int argc, char* argv[])
{
    if(argc != 2) // 如果参数传递不为两个，则提醒使用者，并且退出程序
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]); // 将端口号类型转换为uint16_t
    unique_ptr<udpServer> udper(new udpServer(handlerMessage, port)); // 创建服务端对象
    udper->initServer();
    udper->start();
    return 0;
}
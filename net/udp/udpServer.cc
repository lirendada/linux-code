#include "udpServer.hpp"
#include <memory>
using namespace std;
using namespace Server;

static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

int main(int argc, char* argv[])
{
    if(argc != 2) // 如果参数传递不为两个，则提醒使用者，并且退出程序
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }

    uint16_t port = atoi(argv[1]); // 将端口号类型转换为uint16_t
    unique_ptr<udpServer> udper(new udpServer(port)); // 创建服务端对象

    udper->initServer();
    udper->start();

    return 0;
}
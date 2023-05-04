#include "udpClient.hpp"
#include <memory>
using namespace Client;

static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " destination_ip destination_port\n\n";
}

int main(int argc, char* argv[])
{
    if(argc != 3) // 如果参数传递不为两个，则提醒使用者，并且退出程序
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }

    string destination_ip = argv[1];
    uint16_t destination_port = atoi(argv[2]);
    unique_ptr<udpClient> udper(new udpClient(destination_ip, destination_port));

    udper->initClient();
    udper->run();
    return 0;
}
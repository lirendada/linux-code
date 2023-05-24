#include "tcpclient.hpp"
#include <memory>
using namespace Client;

void Usage(string proc)
{
    cout << "Please follow the true usage:\n\t" << proc << " dest_ip dest_port\n\n";
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }
    string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    unique_ptr<tcpClient> tcpclient(new tcpClient(ip, port));
    tcpclient->initClient();
    tcpclient->run();
    return 0;
}
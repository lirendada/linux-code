#include "tcpserver.hpp"
#include "memory"
using namespace Server;

void Usage(string proc)
{
    cout << "Please follow the true usage:\n\t" << proc << " server_port\n";
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    uint16_t port = atoi(argv[1]);

    unique_ptr<tcpServer> tcpserver(new tcpServer(port));
    tcpserver->initServer();
    tcpserver->start();
    return 0;
}
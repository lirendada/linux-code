#include "../source/server.hpp"

int main()
{
    Socket cli_sock;
    cli_sock.create_client(8080, "127.0.0.1");
    for (int i = 0; i < 5; i++) 
    {
        std::string str = "hha";
        cli_sock.Send(str.c_str(), str.size());
        char buf[1024] = {0};
        cli_sock.Recv(buf, 1023);
        DLOG("%s", buf);
        sleep(1);
    }
    while(1) sleep(1);
    return 0;
}
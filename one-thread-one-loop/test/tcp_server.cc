#include "../source/server.hpp"

int main()
{
    Socket server;
    server.create_server(8080);
    while(true)
    {
        int newfd = server.Accept();
        if(newfd != -1)
        {
            Socket client(newfd);
            char buf[1024] = {0};
            client.Recv(buf, 1023);
            DLOG("%s", buf);
            sleep(1);

            std::string str = "hello lirendada!";
            client.Send(str.c_str(), str.size());
            client.Close();
        }
        sleep(1);
    }
    server.Close();
    return 0;
}
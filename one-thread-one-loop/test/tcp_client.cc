#include "../source/server.hpp"

int main()
{
    // 创建客户端套接字
    Socket client_sock;
    client_sock.create_client(8080, "127.0.0.1");

    // 做五次简单的发送和回响，所以会刷新五次连接
    for(int i = 0; i < 5; ++i)
    {
        std::string str = "lirendada";
        client_sock.Send(str.c_str(), str.size());

        char buf[1024] = { 0 };
        client_sock.Recv(buf, sizeof(buf) - 1);
        DLOG("%s", buf);
        sleep(1);
    }

    // 进入死循环
    while(1) sleep(1);
    client_sock.Close();
    return 0;
}
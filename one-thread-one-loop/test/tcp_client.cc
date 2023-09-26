#include "../source/server.hpp"

int main()
{
    // 创建客户端套接字
    Socket client_sock;
    client_sock.create_client(8080, "127.0.0.1");
    while(true)
    {
        // 做一个简单的发送和回响
        std::string str;
        getline(std::cin, str);
        client_sock.Send(str.c_str(), str.size());

        char buf[1024] = { 0 };
        client_sock.Recv(buf, sizeof(buf) - 1);
        DLOG("%s", buf);
        sleep(1);
    }
    client_sock.Close();
    return 0;
}
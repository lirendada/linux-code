#include "../source/server.hpp"

// 测试长连接的demo
int main()
{
    // 创建客户端套接字
    Socket client_sock;
    client_sock.create_client(8080, "81.71.97.127");

    std::string str = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(true)
    { 
        assert(client_sock.Send(str.c_str(), str.size()) != -1);
        char buf[1024] = { 0 };
        client_sock.Recv(buf, sizeof(buf) - 1);
        DLOG("%s", buf);
        sleep(15);
    }
    return 0;
}
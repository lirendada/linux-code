#include "../source/server.hpp"

/* 给服务器发送一个数据，告诉服务器要发送1024字节的数据，但是实际发送的数据不足1024，查看服务器处理结果 */
/*
    1. 如果数据只发送一次，服务器将得不到完整请求，就不会进行业务处理，客户端也就得不到响应，最终超时关闭连接
    2. 连着给服务器发送了多次小的请求，服务器会将后边的请求当作前边请求的正文进行处理，而后处理的时候有可能就会因为处理错误而关闭连接
*/
int main()
{
    // 创建客户端套接字
    Socket client_sock;
    client_sock.create_client(8080, "81.71.97.127");

    std::string str = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\n\r\nlirendada";
    while(true)
    { 
        assert(client_sock.Send(str.c_str(), str.size()) != -1);
        assert(client_sock.Send(str.c_str(), str.size()) != -1);
        char buf[1024] = { 0 };
        client_sock.Recv(buf, sizeof(buf) - 1);
        DLOG("%s", buf);
        sleep(1);
    }
    return 0;
}
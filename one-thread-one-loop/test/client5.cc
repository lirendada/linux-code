#include "../source/http/httpserver.hpp"

/* 大文件传输测试，给服务器上传一个大文件，服务器将文件保存下来，观察处理结果（上传的文件，应该和服务器保存的文件一致才对） */
int main()
{
    // 创建客户端套接字
    Socket client_sock;
    client_sock.create_client(8080, "81.71.97.127");

    std::string str = "put /1234.txt HTTP/1.1\r\nConnection: keep-alive\r\n\r\n";
    std::string body;
    Util::read_file("./test.txt", &body);
    str += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    assert(client_sock.Send(str.c_str(), str.size()) != -1);
    assert(client_sock.Send(body.c_str(), body.size()) != -1);

    char buf[4096] = { 0 };
    client_sock.Recv(buf, sizeof(buf) - 1);
    DLOG("%s", buf);
    sleep(3);
    return 0;
}
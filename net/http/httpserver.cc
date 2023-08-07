#include "httpserver.hpp"
#include "util.hpp"
#include "protocol.hpp"
#include <memory>
#include <string>
void Usage(string proc)
{
    cout << "Usage: \n\t" << proc << " port\n\r";
}

// 请求资源的函数
bool Get(int socketfd, const httpRequest& req, httpResponse& resp)
{
    // 1. 请求处理 -- 因为已经处理完毕，只需要打印即可
    cout << "+++++++++++++++++http start++++++++++++++++++" << endl;
    cout << req.inbuffer << endl;

    cout << "\r\n下面是解析出来的一些变量：" << endl;
    cout << "path: " << req.path << endl;
    cout << "suffix: " << req.suffix << endl;
    cout << "size: " << req.size << "字节" << endl;
    cout << "++++++++++++++++++http end+++++++++++++++++++" << endl;


    // 2. 响应处理
    // string respline = "HTTP/1.1 307 Temporary Redirect\r\n";
    string respline = "HTTP/1.1 200 Accept\r\n"; 	// 响应行，这里设为成功
    string resphead = Util::suffixHash(req.suffix); // 这里响应报头通过suffixHash函数获取其对应的返回资源，如html
    if(req.size > 0)
    {
        resphead += "Content-Length: ";
        resphead += to_string(req.size);
        resphead += sep;
    }

    // 可以携带Location字段，达到重定向的目的
    // resphead += "Location: https://lirendada.github.io/\r\n";

    // 往后20秒，每次http请求时候都会自动携带曾经设置的所有cookie，帮助服务器进行鉴权行为 -- http会话保持
    resphead += "Set-Cookie: name=lirendada; Max-Age=20\r\n"; 

    string empty = "\r\n"; // 空行
    string body;
    body.resize(req.size); // 记得要先开辟空间
    if(!Util::readFile(req.path, (char*)body.c_str(), req.size)) // 读取对应的资源文件
    {
        // 若读取不到读取资源，则读取错误页面，这个肯定会成功 
        // 注意要计算错误页面的大小，然后重新开辟空间
        struct stat st;
        stat(html_404.c_str(), &st);
        resphead += "Content-Length: ";
        resphead += to_string(st.st_size);
        resphead += sep;
        body.resize(st.st_size);
        Util::readFile(html_404, (char*)body.c_str(), st.st_size); 
    }

    resp.outbuffer = respline + resphead + empty; // 将响应的各部分组织起来成为一个完整的报文
    
    cout << "----------------------http response start---------------------------" << endl;
    std::cout << resp.outbuffer << std::endl;
    cout << "----------------------http response end-----------------------------" << endl;
    resp.outbuffer += body;
    return true;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    uint16_t port = atoi(argv[1]);
    unique_ptr<httpserver> httper(new httpserver(Get, port));
    httper->start();
    httper->run();
    return 0;
}
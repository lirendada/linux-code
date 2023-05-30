#include "httpserver.hpp"
#include <memory>
#include <string>
void Usage(string proc)
{
    cout << "Usage: \n\t" << proc << " port\n\r";
}

// 将请求文件的格式转化为响应的格式
string suffixHash(const string& suffix)
{
    std::string ct = "Content-Type: ";

    if (suffix == ".html")
        ct += "text/html";
    else if (suffix == ".jpg")
        ct += "application/x-jpg;image/jpeg";

    ct += "\r\n";
    return ct;
}

// 1. 服务器和网页分离，html
// 2. url -> / : web根目录
// 3. 我们要正确的给客户端返回资源类型，我们首先要自己知道！所有的资源都有后缀！！
bool Get(int socketfd, const httpRequest& req, httpResponse& resp)
{
    // if(req.path == "test.py")
    // {
    //     //建立进程间通信，pipe
    //     //fork创建子进程，execl("/bin/python", test.py)
    //     // 父进程，将req.parm 通过管道写给某些后端语言，py，java，php等语言
    // }
    // if(req.path == "/search")
    // {
    //     // req.parm
    //     // 使用我们自己写的C++的方法，提供服务
    // }

    cout << "+++++++++++++++++http start++++++++++++++++++" << endl;
    cout << req.inbuffer << endl;
    cout << "method: " << req.method << endl;
    cout << "url: " << req.url << endl;
    cout << "httpversion: " << req.httpversion << endl;
    cout << "path: " << req.path << endl;
    cout << "suffix: " << req.suffix << endl;
    cout << "size: " << req.size << "字节" << endl;
    cout << "++++++++++++++++++http end+++++++++++++++++++" << endl;

    // string resphead = "HTTP/1.1 307 Temporary Redirect\r\n";
    string respline = "HTTP/1.1 200 Accept\r\n";
    string resphead = suffixHash(req.suffix);
    if(req.size > 0)
    {
        resphead += "Content-Length: ";
        resphead += to_string(req.size);
        resphead += sep;
    }

    resphead += "Set-Cookie: name=lirendada; Max-Age=60\r\n"; // 往后60秒，每次http请求时候都会自动携带曾经设置的所有cookie，帮助服务器进行鉴权行为 -- http会话保持
    // resphead += "Location: https://lirendada.github.io/\r\n";
    string empty = "\r\n";
    
    // string body = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>sb鸡哥</h1></head><body><p>北京交通广播《一路畅通》“交通大家谈”节目，特邀北京市交通委员会地面公交运营管理处处长赵震、北京市公安局公安交通管理局秩序处副处长 林志勇、北京交通发展研究院交通规划所所长 刘雪杰为您解答公交车专用道6月1日起社会车辆进出公交车道须注意哪些？</p></body></html>";

    string body;
    body.resize(req.size); // 记得要开辟空间
    if(!Util::readFile(req.path, (char*)body.c_str(), req.size)) // 若读取不到读取的资源
    {
        Util::readFile(html_404, (char*)body.c_str(), req.size); // 则读取错误页面，这个肯定会超过
    }
    // if(!Util::readFile(req.path, &body))
    // {
    //     Util::readFile(html_404, &body);
    // }

    resp.outbuffer = respline + resphead + empty;
    cout << "----------------------http response start---------------------------" << endl;
    std::cout << resp.outbuffer << std::endl;
    cout << "----------------------http response end---------------------------" << endl;
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
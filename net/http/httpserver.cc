#include "httpserver.hpp"
#include <memory>
#include <string>
void Usage(string proc)
{
    cout << "Usage: \n\t" << proc << " port\n\r";
}

bool Get(int socketfd, const httpRequest& req, httpResponse& resp)
{
    cout << "+++++++++++++++++http start++++++++++++++++++" << endl;
    cout << req.inbuffer << endl;
    cout << "method: " << req.method << endl;
    cout << "url: " << req.url << endl;
    cout << "httpversion: " << req.httpversion << endl;
    cout << "path: " << req.path << endl;
    cout << "++++++++++++++++++http end+++++++++++++++++++" << endl;

    string reqline = "HTTP/1.1 202 Accepted\r\n";
    string reshead = "Content-Type: text/html\r\n";
    string empty = "\r\n";
    string body = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>sb鸡哥</h1></head><body><p>北京交通广播《一路畅通》“交通大家谈”节目，特邀北京市交通委员会地面公交运营管理处处长赵震、北京市公安局公安交通管理局秩序处副处长 林志勇、北京交通发展研究院交通规划所所长 刘雪杰为您解答公交车专用道6月1日起社会车辆进出公交车道须注意哪些？</p></body></html>";

    resp.outbuffer = reqline + reshead + empty + body;
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
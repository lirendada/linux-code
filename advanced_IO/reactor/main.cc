#include "reactor.hpp"
#include "threadpool.hpp"
#include <memory>
using namespace std;

static void Usage(const string& proc)
{
    cerr << "\nUsage:\n\t" << proc << " port\n\n"; 
}

// req: 里面一定是我们的处理好的一个完整的请求对象
// resp: 根据req，进行业务处理填充resp，不用管理任何读取和写入、序列化和反序列化等任何细节
bool cal(const Request &req, Response &resp)
{
    // req已经有结构化完成的数据了，可以直接使用
    resp._exitcode = OK;
    resp._result = OK;

    switch(req._op)
    {
    case '+':
        resp._result = req._x + req._y;
        break;
    case '-':
        resp._result = req._x - req._y;
        break;
    case '*':
        resp._result = req._x * req._y;
        break;
    case '/':
    {
        if(req._y == 0)
            resp._exitcode = DIV_ZERO;
        else
            resp._result = req._x / req._y;
    }
        break;
    case '%':
    {
        if(req._y == 0)
            resp._exitcode = MOD_ZERO;
        else
            resp._result = req._x % req._y;
    }
        break;
    default:
        resp._exitcode = OP_ERROR;
        break;
    }

    return true;
}

// 业务处理函数
void service(connection* conn)
{
    std::string text;
    while(recvPackage(conn->_inbuffer, &text))
    {
        logMessage(Level::DEBUG, "service while begin");
        // 1. 去报头
        std::string body;
        if(!delRule(text, &body))
            return;
        std::cout << "去掉报头的正文：\n" << body << std::endl;

        // 2. 反序列化
        Request req;
        if(!req.deserialize(body))
            return;
        
        // 3. 业务处理
        Response resp;
        cal(req, resp);

        // 4. 序列化
        std::string out;
        if(!resp.serialize(&out))
            return;
        
        // 5. 添加报头
        // 6. 将其放到输出缓冲区中
        conn->_outbuffer += addRule(out);

        logMessage(Level::DEBUG, "service while end, %s", conn->_outbuffer.c_str());
    }
    // 7. 收到完整报文之后，调用发送函数进行响应
    if(conn->_sender)
        conn->_sender(conn);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }

    unique_ptr<reactor> server(new reactor(service, atoi(argv[1])));
    server->run();
    return 0;
}
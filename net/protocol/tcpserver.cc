#include "tcpserver.hpp"
#include <memory>
using namespace Server;

void Usage(string proc)
{
    cout << "Please follow the true usage:\n\t" << proc << " server_port\n";
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

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    uint16_t port = atoi(argv[1]);

    unique_ptr<tcpServer> tcpserver(new tcpServer(port));
    tcpserver->initServer();
    tcpserver->start(cal);
    return 0;
}
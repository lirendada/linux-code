#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>
using namespace std;

const char* SEP = " ";                 // 分隔符，用于区分开序列化之间的字符
const int SEP_LEN = strlen(SEP);
const char* SEP_LINE = "\r\n";         // 行分隔符，用于区分开各报文之间
const int SEP_LINE_LEN = strlen(SEP_LINE);

enum{
    OK,
    DIV_ZERO,
    MOD_ZERO,
    OP_ERROR
};

// 接收客户端或者服务端发来的报文，该函数的目的是拿到一个完整的报文
bool recvPackage(int sockfd, string& recvbuffer, string* recv_string)
{
    // 为了让接收到的数据出了函数后不丢失，使用static来修饰缓冲区
    // 不过这里也可以选择让recvbuffer被外部所持有，读取时传进来即可
    // static string recvbuffer; 
    
    char tmpbuffer[1024];
    while(true)
    {
        ssize_t n = recv(sockfd, tmpbuffer, sizeof(tmpbuffer) - 1, 0);
        if(n > 0)
        {
            tmpbuffer[n] = 0;
            recvbuffer += tmpbuffer; // 进行尾插到缓冲区
            
            // 分析处理
            auto pos = recvbuffer.find(SEP_LINE);
            if(pos == string::npos)
                continue; // 这步非常的关键，当没读到\r\n的时候说明读取的报文还不完整，则继续读而不是直接退出
            
            // 到了此处说明起码拿到了包头
            // 而我们定义包头的规则就是存放正文的长度
            int body_size = stoi(recvbuffer.substr(0, pos));

            // 此时有可能正文还没有接收完整，我们要另作判断
            // 如果此时 pos + SEP_LINE_LEN*2 + body_size <= recvbuffer.size() 的话
            // 说明至少我们接收到了一个完整的报文！
            // 如果不是的话，我们要继续循环去读取
            int package_size = pos + SEP_LINE_LEN*2 + body_size;
            std::cout << "处理前#recvbuffer: \n" << recvbuffer << std::endl;
            if(package_size > recvbuffer.size())
            {
                cout << "你输入的消息，没有严格遵守我们的协议，正在等待后续的内容, continue" << endl;
                continue; // 重新接收，直到读到一个完整的报文为止
            }

            // 走到这说明起码读到一个完整的报文
            // 那么就把它拿到，并且将其重缓冲区中删去
            *recv_string = recvbuffer.substr(0, package_size);
            recvbuffer.erase(0, package_size);

            std::cout << "处理后#recvbuffer:\n " << recvbuffer << std::endl;
            break;
        }
        else
        {
            // 异常或者客户端不写了，则退出
            return false;
        }
    }
    return true;
}

// 为报文添加自定义首部和尾部的函数
// 自定义首部规则：报文长度+行分隔符"\r\n"，比如下面的：
// "x or yyyy"  -->  "有效载荷长度"\r\n"x or yyyy"\r\n
// "exitcode result"  -->  "有效载荷长度"\r\n"exitcode result"\r\n
// 其中有效载荷指的就是"x or yyyy"和"exitcode result"
string addRule(const string& body)
{
    string send_string = to_string(body.size()); // 有效载荷长度
    send_string += SEP_LINE; // 加上行分隔符
    send_string += body;     // 加上有效载荷
    send_string += SEP_LINE; // 加上行分隔符

    return send_string;
}

// 为报文去掉自定义首部和尾部的函数，比如：
// "有效载荷长度"\r\n"x or yyyy"\r\n  -->  "x or yyyy"
bool delRule(const string& package, string* body)
{
    auto pos = package.find(SEP_LINE);
    if(pos == string::npos)
        return false;
    int body_size = stoi(package.substr(0, pos));
    *body = package.substr(pos + SEP_LINE_LEN, body_size);
    return true;
}

// 请求一般是客户端给服务端的
class Request
{
public:
    Request()
        :_x(0), _y(0), _op(0)
    {}
    Request(int x, int y, int op)
        :_x(x), _y(y), _op(op)
    {}

    //   序列化相当于：结构体 -》 字符串（更正确的说法是字节流）
    // 反序列化相当于：字符串 -》 结构体
    // 1. 自己实现
    // 2. 用现成的库函数
    bool serialize(string* out) // 输出型参数
    {
#ifdef MYSELF
        // 结构体 -> "x op y"
        *out = ""; // 格式化一下out

        *out += to_string(_x);
        *out += SEP;
        *out += _op;
        *out += SEP;
        *out += to_string(_y);
#else
        // 填写键值对
        Json::Value root;
        root["first"] = _x;
        root["second"] = _y;
        root["operator"] = _op;

        // 选择方式进行序列化
        Json::FastWriter writer;
        // Json::StyledWriter writer;
        *out = writer.write(root);
#endif
        return true;
    }
    bool deserialize(const string& in)
    {
#ifdef MYSELF
        // "x op yyyy" -> 结构体
        // 1.首先找到两个分隔符的位置
        auto left = in.find(SEP);
        auto right = in.rfind(SEP);
        if(left == string::npos || right == string::npos || left == right)
            return false;
        if(right - (left+SEP_LEN) != 1) // 规定分隔符必须是一个字符的长度
            return false;

        // 获取x和y的字符串，判断是否为空
        string x_string = in.substr(0, left);
        string y_string = in.substr(right + SEP_LEN);
        if(x_string.empty() || y_string.empty())
            return false;

        // 给结构体赋值
        _x = stoi(x_string);
        _y = stoi(y_string);
        _op = in[left+SEP_LEN];
#else
        Json::Reader reader;
        Json::Value root;
        reader.parse(in, root); // 将in中的字符流反序列化到root中

        _x = root["first"].asInt();
        _y = root["second"].asInt();
        _op = root["operator"].asInt(); // 注意这里因为字符也是ASCII码，所以直接转为int即可
#endif
        return true;
    }
public:
    int _x;
    int _y;
    char _op;
};

// 响应一般是服务端发给客户端的
class Response
{
public:
    Response()
        :_exitcode(0), _result(0)
    {}
    Response(int exitcode, int result)
        :_exitcode(exitcode), _result(result)
    {}

    // 1. 自己实现
    // 2. 用现成的库函数
    bool serialize(string* out) // 输出型参数
    {
#ifdef MYSELF
        // 结构体 -》 "exitcode result"
        *out = ""; // 格式化一下字符串
        // 将类内成员转化为字符串
        *out += to_string(_exitcode);
        *out += SEP;
        *out += to_string(_result);
#else
        Json::Value root;
        root["exitcode"] = _exitcode;
        root["result"] = _result;

        Json::FastWriter writer;
        *out = writer.write(root);
#endif
        return true;
    }
    bool deserialize(const string& in)
    {
#ifdef MYSELF
        // "exitcode result" -》 结构体
        auto pos = in.find(SEP);
        if(pos == string::npos)
            return false;
        string ec_string = in.substr(0, pos);
        string res_string = in.substr(pos + SEP_LEN);
        if(ec_string.empty() || res_string.empty())
            return false;

        _exitcode = stoi(ec_string);
        _result = stoi(res_string);
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);
        
        _exitcode = root["exitcode"].asInt();
        _result = root["result"].asInt();
#endif
        return true;
    }
public:
    int _exitcode;
    int _result;
};
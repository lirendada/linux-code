#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>
#include "err.hpp"
#include "connection.hpp"
using namespace std;

const char* SEP = " ";                 // 分隔符，用于区分开序列化之间的字符
const int SEP_LEN = strlen(SEP);
const char* SEP_LINE = "\r\n";         // 行分隔符，用于区分开各报文之间
const int SEP_LINE_LEN = strlen(SEP_LINE);


// 接收客户端或者服务端发来的报文，该函数的目的是拿到一个完整的报文
bool recvPackage(std::string &inbuffer, std::string *recv_string)
{
    *recv_string = "";
    // 分析处理
    auto pos = inbuffer.find(SEP_LINE);
    if (pos == std::string::npos)
        return false;

    logMessage(Level::DEBUG, "%d", stoi(inbuffer.substr(0, pos)));
    int body_size = stoi(inbuffer.substr(0, pos)); 
    int package_size = pos + SEP_LINE_LEN*2 + body_size;
    if(package_size > inbuffer.size())
        return false;

    // 至少有一个完整的报文
    *recv_string = inbuffer.substr(0, package_size);
    logMessage(Level::DEBUG, "%s", recv_string->c_str());
    inbuffer.erase(0, package_size);
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
    int _x;
    int _y;  
    char _op; // 运算符
public:
    Request(int x = 0, int y = 0, int op = 0)
        :_x(x), _y(y), _op(op)
    {}

    // 序列化相当于：  结构体 -》 字符串（更正确的说法是字节流）
    // 反序列化相当于：字符串 -》 结构体
    bool serialize(string* out) // 输出型参数
    {
        // 填写键值对
        Json::Value root;
        root["first"] = _x;
        root["second"] = _y;
        root["operator"] = _op;

        // 选择方式进行序列化
        Json::FastWriter writer;
        *out = writer.write(root);
        return true;
    }
    bool deserialize(const string& in)
    {
        Json::Reader reader;
        Json::Value root;
        reader.parse(in, root); // 将in中的字符流反序列化到root中

        _x = root["first"].asInt();
        _y = root["second"].asInt();
        _op = root["operator"].asInt(); // 注意这里因为字符也是ASCII码，所以直接转为int即可
        return true;
    }
};

// 响应一般是服务端发给客户端的
class Response
{
public:
    int _exitcode = 0; // 退出码，规定0表示计算成功，非0表示计算失败
    int _result = 0;   // 计算结果
public:
    Response(int exitcode = 0, int result = 0)
        :_exitcode(exitcode), _result(result)
    {}

    bool serialize(string* out) // 输出型参数
    {
        Json::Value root;
        root["exitcode"] = _exitcode;
        root["result"] = _result;

        Json::FastWriter writer;
        *out = writer.write(root);
        return true;
    }
    bool deserialize(const string& in)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);
        
        _exitcode = root["exitcode"].asInt();
        _result = root["result"].asInt();
        return true;
    }
};


// 将字符串转化为请求结构体的工具函数
bool get_req_from_string(const string& msg, Request& req)
{
    string leftnum, rightnum;
    char op;
    int status = 0; // 0表示当前为左操作数范围，1表示当前为右操作数范围
    bool op_occur = false;
    for(int i = 0; i < msg.size(); ++i)
    {
        if(!isdigit(msg[i])) // 非数字的情况
        {
            // 如果不是操作符则直接false
            if(msg[i] != '+' && msg[i] != '-' && msg[i] != '*' && msg[i] != '/' && msg[i] != '%')
                return false;

            op_occur = true; // 标志出现过操作符
            op = msg[i];
            status = 1;      // 变成右操作数范围
            continue;
        }
        if(status == 0)
            leftnum += msg[i];
        else
            rightnum += msg[i];
    }

    // 如果没出现过操作符直接false
    if(!op_occur)
        return false;

    req._x = stoi(leftnum), req._y = stoi(rightnum), req._op = op;
    return true;
}

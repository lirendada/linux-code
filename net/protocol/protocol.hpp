#pragma once
#include <iostream>
#include <string>
#include <cstring>
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

// 为报文添加自定义首部和尾部的函数
// 自定义首部规则：报文长度+行分隔符"\r\n"
string addRule(const string& body)
{

}

// 为报文去掉自定义首部和尾部的函数
string delRule(const string& package)
{

}

// 请求一般是接收的
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
        // 结构体 -> "x op y"
        *out = ""; // 格式化一下out

        *out += to_string(_x);
        *out += SEP;
        *out += _op;
        *out += SEP;
        *out += to_string(_y);
        return true;
    }
    bool deserialize(const string& in)
    {
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
        _op = in[right - (left+SEP_LEN)];
        return true;
    }
public:
    int _x;
    int _y;
    char _op;
};

// 响应一般是发送出去的
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
        // 结构体 -》 "exitcode result"
        *out = ""; // 格式化一下字符串
        // 将类内成员转化为字符串
        *out += to_string(_exitcode);
        *out += SEP;
        *out += to_string(_result);

        return true;
    }
    bool deserialize(const string& in)
    {
        // "exitcode result" -》 结构体
        auto pos = in.find(SEP);
        if(pos == string::npos)
            return false;
        _exitcode = stoi(in.substr(0, pos));
        _result = stoi(in.substr(pos + 1, in.size() - pos - SEP_LEN));

        return true;
    }
public:
    int _exitcode;
    int _result;
};
#pragma once
#include <iostream>
#include <functional>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <fstream>

class CalTask
{
    using func_t = std::function<int(int, int, char)>;
public:
    CalTask()
    {}
    CalTask(int x, int y, char op, func_t callback)
        :_x(x), _y(y), _op(op), _callback(callback)
    {}
    std::string operator()()
    {
        int result = _callback(_x, _y, _op);
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = %d", _x, _op, _y, result);
        return buffer;
    }
    std::string toTaskString()
    {
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = ?", _x, _op, _y);
        return buffer;
    }
private:
    int _x;
    int _y;
    char _op;
    func_t _callback;
};

const std::string oper = "+-*/";

int caltask(int x, int y, char op)
{
    int result = 0;
    switch(op)
    {
    case '+':
        result = x + y;
        break;
    case '-':
        result = x - y;
        break;
    case '*':
        result = x * y;
        break;
    case '/':
    {
        if(y == 0)
        {
            std::cerr << "除零错误" << std::endl;
            result = -1;
        }
        else
        {
            result = x / y;
        }
    }
        break;
    default:
        break;
    }
    return result;
}


class SaveTask
{
    using func_t = std::function<void(const std::string&)>;
public:
    SaveTask()
    {}
    SaveTask(const std::string &message, func_t callback)
        : _message(message), _callback(callback)
    {}
    void operator()()
    {
        _callback(_message);
    }
private:
    std::string _message;
    func_t _callback;
};

void savetask(const std::string& message)
{
    // c++的文件读写方式
    std::fstream fs("./log.txt", std::fstream::out | std::fstream::app);
    fs << message << "\n";
    fs.close();
}
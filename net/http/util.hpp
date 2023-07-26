#pragma once
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class Util
{
public:
    // 获取请求行字符串的函数
    static string getReqLine(string& buffer, const string& sep)
    {
        auto pos = buffer.find(sep);
        if(pos == string::npos)
            return "";
        string tmp = buffer.substr(0, pos);
        return tmp;
    }

    // 读取资源文件的函数
    static bool readFile(const string& resource, char* buffer, int size)
    {
        ifstream in(resource, ios::binary);
        if(!in.is_open())  
            return false; // resource not found
            
        in.read(buffer, size);
        
        // 注意不能按下面这种形式读取图片，因为图片是二进制形式
        // string line;
        // while(getline(in, line)) // 行读取
        // {
        //     *out += line;
        // }

        in.close();
        return true;
    }

    // 将请求文件的格式转化为http对应的响应格式
    static string suffixHash(const string& suffix)
    {
        std::string ct = "Content-Type: ";

        if (suffix == ".html")
            ct += "text/html";
        else if (suffix == ".jpg")
            ct += "application/x-jpg;image/jpeg";
        else if(suffix == ".ico")
            ct += "image/x-icon";

        ct += "\r\n";
        return ct;
    }
};
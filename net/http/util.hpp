#pragma once
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

class Util
{
public:
    static string getOneLine(string& buffer, const string& sep)
    {
        auto pos = buffer.find(sep);
        if(pos == string::npos)
            return "";
        string tmp = buffer.substr(0, pos);
        buffer.erase(0, tmp.size() + sep.size());
        return tmp;
    }
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
};
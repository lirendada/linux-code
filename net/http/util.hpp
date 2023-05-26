#pragma once
#include <iostream>
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
};
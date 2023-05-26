#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include "util.hpp"
using namespace std;
const string sep = "\r\n";
const string default_path = "./wwwroot";
const string home_page = "index.html";

class httpRequest
{
public:
    void parse()
    {
        // 1.从inbuffer中拿到第一行也就是请求行，其中分隔符为"\r\n"
        string line = Util::getOneLine(inbuffer, sep);
        if(line.empty())
            return;

        // 2.从请求行中提取出三个字段
        // 直接使用stringstream的流插入，它以空格为标识符进行分割，符合我们预期
        stringstream sstr(line);
        sstr >> method >> url >> httpversion; 

        // 3.添加web的默认路径
        path += default_path; // 变成./wwwroot
        path += url;          // url可能是web根目录，有可能是具体的路径
        if(path[path.size() - 1] == '/')
            path += home_page; // 如果url是web根目录，则让其加载首页
    }
public:
    string inbuffer;

    string method;
    string url;
    string httpversion;

    string path; // 默认路径
};

class httpResponse
{
public:
    string outbuffer;
};
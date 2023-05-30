#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.hpp"
using namespace std;
const string sep = "\r\n";
const string default_path = "./wwwroot";
const string home_page = "index.html";
const string html_404 = "wwwroot/404.html";

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

        // 2.1 /test.py?name=zhangsan&pwd=12345
        // 通过？将左右进行分离
        // 如果是POST方法，本来就是分离的！
        // 左边PATH， 右边parm

        // 3.添加web的默认路径
        path += default_path; // 变成./wwwroot
        path += url;          // url可能是web根目录，有可能是具体的路径
        // 如果url是web根目录，则让其加载首页，即这里的./wwwroot/index.html
        // 其实这里实现的不是很好，只是个样例！
        if(path[path.size() - 1] == '/')
            path += home_page; 
        
        // 4. 获取请求文件的后缀，注意包括这个分隔符"."
        auto pos = path.rfind('.');
        if(pos == string::npos)
            suffix = ".html";
        else
            suffix = path.substr(pos);
        
        // 5. 获取资源的大小
        struct stat st;
        int n = stat(path.c_str(), &st);
        if(n == 0)
            size = st.st_size;
        else
            size = 0; // 注意设为0比较好，因为如果设为-1的话，整个文件大小就要减一，开辟空间的时候就要多加一，而0就不用
    }
public:
    string inbuffer;

    string method;     
    string url;     
    string httpversion; 
    string path;   // 访问路径
    string suffix; // 访问文件的后缀格式
    int size;      // 请求资源大小
};

class httpResponse
{
public:
    string outbuffer;
};
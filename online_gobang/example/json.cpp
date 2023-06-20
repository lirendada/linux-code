#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <jsoncpp/json/json.h>

// 序列化
std::string serialize()
{
    // 1.实例化json::value对象，将要序列化的数据存储到该对象中
    Json::Value root;
    root["姓名"] = "liren";
    root["年龄"] = 1314;
    root["成绩"].append(100);
    root["成绩"].append(200);
    root["成绩"].append(300);

    // 2.实例化工厂类对象StreamWriterBuilder
    Json::StreamWriterBuilder swb;

    // 3.通过 StreamWriterBuilder 来生产一个 StreamWriter 对象，最好用智能指针管理
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());

    // 4.通过 StreamWriter 对象实现序列化
    std::stringstream ss;
    int n = sw->write(root, &ss);
    if(n != 0)
    {
        std::cout << "json serialize fail!" << std::endl;
        return "";
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

void unserialize(const std::string& str)
{
    // 1. 实例化一个CharReaderBuilder工厂类对象
    Json::CharReaderBuilder crb;

    // 2. 使用CharReaderBuilder工厂类来生产一个CharReader类对象，最好用智能指针管理
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());

    // 3. 定义一个Json::Value类对象存储反序列化后的数据
    Json::Value root;

    // 4. 使用CharReader类对象进行json格式字符串str的反序列化
    std::string err;
    bool n = cr->parse(str.c_str(), str.c_str() + str.size(), &root, &err);
    if(n == false)
    {
        std::cout << "json unserialize fail!" << std::endl;
        return;
    }

    // 5. 逐个元素去访问Json::Value中的数据，记得要用转换函数进行转换类型
    std::cout << root["姓名"].asString() << std::endl;
    std::cout << root["年龄"].asInt() << std::endl;
    int size = root["成绩"].size();
    for(int i = 0; i < size; ++i)
    {
        std::cout << root["成绩"][i].asInt() << std::endl;
    }
}

int main()
{
    std::string str = serialize();
    unserialize(str);
    return 0;
}
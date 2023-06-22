#include "logger.hpp"
#include "util.hpp"

#define HOST "127.0.0.1"  // 不允许使用公网地址，所以用本地换回
#define USER "root"
#define PASSWD ""
#define DBNAME "gobang"
#define PORT 3306

void mysql_test()
{
    MYSQL* mysql = mysql_util::mysql_create(HOST, USER, PASSWD, DBNAME, PORT);
    const char* query = "insert stu value(null, '利刃', 1314, 100, 150, 149);";
    bool ret = mysql_util::mysql_exec(mysql, query);
    if(ret == false)
        return;
    mysql_util::mysql_destroy(mysql);
}

void json_test()
{
    // 序列化
    Json::Value root;
    root["姓名"] = "liren";
    root["年龄"] = 1314;
    root["成绩"].append(100);
    root["成绩"].append(200);
    root["成绩"].append(300);
    std::string str;
    json_util::serialize(root, str);
    DLOG("%s", str.c_str());

    // 反序列化
    Json::Value newroot;
    json_util::unserialize(str, newroot);
    std::cout << newroot["姓名"].asString() << std::endl;
    std::cout << newroot["年龄"].asInt() << std::endl;
    int size = newroot["成绩"].size();
    for(int i = 0; i < size; ++i)
    {
        std::cout << newroot["成绩"][i].asInt() << std::endl;
    }
}

void str_test() 
{
    std::string str = ",...,,123,234,,,,,345,,,,";
    std::vector<std::string> arry;
    string_util::split(str, ",", arry);
    for (auto s : arry) {
        DLOG("%s", s.c_str());
    }
}

void file_test()
{
    std::string str;
    bool ret = file_util::read("./makefile", str);
    if(ret == false)
        return;
    DLOG("%s", str.c_str());
}

int main()
{
    file_test();
    return 0;
}
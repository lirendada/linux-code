#ifndef __MY_UTIL_H__
#define __MY_UTIL_H__
#include "logger.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
using wsserver_t = websocketpp::server<websocketpp::config::asio>;

// 因为这个头文件的工具都是对外提供服务的接口
// 所以基本都是设置为public和static（就可以不用实例化对象）属性

class mysql_util
{
public:
    // 初始化接口
    static MYSQL* mysql_create(const std::string& host,
                               const std::string& user,
                               const std::string& passwd,
                               const std::string& dbname,
                               uint16_t port = 3306)
    {
        // 1.初始化句柄
        MYSQL* mysql = mysql_init(NULL);
        if(mysql == NULL)
        {
            // 错误处理
            ELOG("init mysql failed");
            return NULL;
        }

        // 2.连接服务器
        if(mysql_real_connect(mysql, 
                           host.c_str(), 
                           user.c_str(), 
                           passwd.c_str(), 
                           dbname.c_str(), 
                           port, 
                           NULL, 0) == NULL)
        {
            // 错误处理
            ELOG("connect mysql server failed : %s", mysql_error(mysql));
            mysql_close(mysql);
            return nullptr;
        }

        // 3.设置字符集
        if(mysql_set_character_set(mysql, "utf8") != 0)
        {
            // 错误处理
            ELOG("mysql set character failed : %s", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }

        // 4.连接选择的数据库 -- 可以不做，但是为了错误处理这里还是进行判断
        if(mysql_select_db(mysql, dbname.c_str()) != 0)
        {
            // 错误处理
            ELOG("mysql select database failed : %s", mysql_error(mysql));
            mysql_close(mysql);
            return NULL;
        }

        // 最后一定一定要记得返回句柄！！！
        return mysql;
    }

    // 执行语句接口
    static bool mysql_exec(MYSQL* mysql, const std::string& query)
    {
        // 先判断句柄是否有效
        if(mysql == NULL)
        {
            ELOG("MYSQL handle invalid");
            return false;
        }
        int n = mysql_query(mysql, query.c_str()); // 执行语句
        if(n != 0)
        {
            // 错误处理
            ELOG("%s --> sql query failed : %s", query.c_str(), mysql_error(mysql));
            return false;
        }
        return true;
    }

    // 释放句柄接口
    static void mysql_destroy(MYSQL* mysql)
    {
        if(mysql != NULL)
            mysql_close(mysql);
    }
};

class json_util
{
public:
    static bool serialize(const Json::Value& root, std::string& out)
    {
        // 1.实例化StreamWriterBuilder工厂类对象
        Json::StreamWriterBuilder swb;

        // 2.通过工厂类对象生产一个StreamWriter对象
        std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
        if(sw.get() == nullptr)
        {
            ELOG("json produce writer failed");
            return false;
        }

        // 3.通过StreamWriter对象进行序列化
        std::stringstream ss;
        int ret = sw->write(root, &ss);
        if(ret != 0)
        {
            // 错误处理
            ELOG("json serialize failed");
            return false;
        }

        // 4.输出型参数赋值
        out = ss.str(); 
        return true;
    }

    static bool unserialize(const std::string& in, Json::Value& root)
    {
        // 1.实例化一个CharReaderBuilder工厂类对象
        Json::CharReaderBuilder crb;

        // 2.通过工厂类对象生产一个CharReader类对象
        std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
        if(cr.get() == nullptr)
        {
            // 错误处理
            ELOG("json produce reader failed");
            return false;
        }

        // 3.通过CharReader类对象进行反序列化
        std::stringstream ss;
        std::string err;
        bool ret = cr->parse(in.c_str(), in.c_str() + in.size(), &root, &err);
        if(ret == false)
        {
            // 错误处理
            ELOG("json unserialize failed");
            return false;
        }
        return true;
    }
};

class string_util
{
public:
    // src：要分割的字符串
    // sep：分隔符
    // res：存放子串的结果集数组
    static int split(const std::string& src, const std::string& sep, std::vector<std::string>& res)
    {
        auto pos = src.find(sep);
        int pre = 0;
        while(pos != std::string::npos)
        {
            // 这个判断是避免sep是','，而字符串是",,,,"的时候，下面在push_back中pos-pre是0，会插入一个空字符串的情况
            if(pos == pre)
            {
                pre += sep.size();
                pos = src.find(sep, pre);
                continue;
            }
            res.push_back(src.substr(pre, pos - pre));      
            // 注意是加sep.size()而不是只加1
            pre = pos + sep.size(); 
            pos = src.find(sep, pre);
        }
        res.push_back(src.substr(pre)); // 别忘了还有最后一个子串
        return res.size();
    }
};

class file_util
{
public:
    static bool read(const std::string& filename, std::string& out)
    {
        // 1.打开文件（注意要用二进制形式打开）
        std::ifstream ifs(filename, std::ios::binary);
        if(ifs.is_open() == false)
        {
            ELOG("%s file open failed", filename.c_str());
            return false;
        }

        // 2.获取文件大小（通过偏移量获取）
        ifs.seekg(0, std::ios::end); // 把文件指针从流的末尾开始偏移0个偏移量，相当于指到末尾
        size_t size = ifs.tellg();   // 读取当前文件指针到开头的偏移量，相当于文件大小
        ifs.seekg(0, std::ios::beg); // 将文件指针重新指向开头
        out.resize(size); // 将out大小进行调整

        // 3.读取文件数据到字符串out中
        //   这里第一个参数传的是首地址，所以要用out[0]的取地址，不能直接使用out.c_str()，因为它是const属性
        ifs.read(&out[0], size);
        if(ifs.good() == false)
        {
            ELOG("read %s file content failed", filename.c_str());
            ifs.close();
            return false;
        }

        // 4.关闭文件
        ifs.close();
        return true;
    }
};

#endif
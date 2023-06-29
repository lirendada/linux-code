#ifndef __MY_DB_H__
#define __MY_DB_H__
#include "util.hpp"
#include <mutex>
#include <cassert>

class user_table
{
private:
    MYSQL* _mysql;   // 操作句柄
    std::mutex _mtx; // 互斥锁保护数据库的操作
public:
    user_table(const std::string& host,
               const std::string& user,
               const std::string& passwd,
               const std::string& dbname,
               uint16_t port = 3306)
    {
        _mysql = mysql_util::mysql_create(host, user, passwd, dbname, port);
        assert(_mysql != NULL);
    }

    ~user_table()
    {
        mysql_util::mysql_destroy(_mysql);
        _mysql = NULL;
    }

    // 注册函数
    bool sign_up(Json::Value& user)
    {
#define SIGN_UP "insert user values(null, '%s', password('%s'), 1000, 0, 0);"
        // 1. 首先判断是否提供了用户名和密码
        if(user["username"].isNull() || user["password"].isNull() || user["username"].asString().empty() || user["password"].asString().empty())
        {
            DLOG("user didn't enter an username or password!");
            return false;
        }

        // 2. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, SIGN_UP, user["username"].asCString(), user["password"].asCString());

        // 3. 执行语句
        bool ret = mysql_util::mysql_exec(_mysql, query);
        if(ret == false)
        {
            DLOG("sign up user failed!");
            return false;
        }
        return true;
    }

    // 登录函数
    bool login(Json::Value& user)
    {
#define LOGIN_SQL "select id, score, total_count, win_count from user where username='%s' and password=password('%s');"
        // 1. 首先判断是否提供了用户名和密码
        if(user["username"].isNull() || user["password"].isNull() || user["username"].asString().empty() || user["password"].asString().empty())
        {
            DLOG("user didn't enter an username or password!");
            return false;
        }

        // 2. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, LOGIN_SQL, user["username"].asCString(), user["password"].asCString());

        // 3. 执行sql语句，因为查询之后需要保存到本地，为了保证保存过程的线程安全，这段代码需要加锁
        //    这里不直接使用加锁，而是通过守卫锁来管理锁，更加安全
        //    并且因为守卫锁是当前作用域有效，所以只对要加锁的区域放在一个空代码块中当作一个作用域
        MYSQL_RES* res = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mtx); // 相当于加锁了
            
             // 执行sql语句
            bool ret = mysql_util::mysql_exec(_mysql, query);
            if(ret == false)
            {
                if(ret == false)
                {
                    DLOG("user login failed!");
                    return false;
                }
            }

            // 保存查询结果到本地
            res = mysql_store_result(_mysql);
            if(res == nullptr)
            {
                DLOG("mysql_store_result!");
                return false;
            }
        }

        // 4. 获取结果集条数，得到结果集 -- 这里行数肯定是只有一行，因为在表中我们规定了用户名是唯一的
        int row_num = mysql_num_rows(res);
        if(row_num == 0)
        {
            DLOG("the user information is not found!");
            return false;
        }
        else if (row_num != 1) 
        {
            DLOG("the user information queried is not unique!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);

        // 5. 将数据库中的用户信息填写到user对象中去，作为输出型参数
        //     这里有细节，因为结果集中的数据都是字符串，所以转化为整型，最好是长整型
        //     但是转化为长整型会报错，所以再强转为json的数据类型，如下面的Json::UInt64
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);

        // 6. 别忘了释放结果集
        mysql_free_result(res);
        return true;
    }

    // 通过用户名获取用户信息
    bool select_by_name(const std::string& name, Json::Value& user)
    {
#define SELECT_BY_NAME "select id, score, total_count, win_count from user where username='%s';"
        // 1. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, SELECT_BY_NAME, name.c_str());

        // 2. 执行语句，并且保存结果集到本地 -- 因为是查询语句，所以还是依然要加锁保证线程安全
        MYSQL_RES* res = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mtx); // 相当于加锁

            // 执行语句
            bool ret = mysql_util::mysql_exec(_mysql, query);
            if(ret == false)
            {
                DLOG("get user by name failed!!");
                return false;
            }

            // 保存结果集
            res = mysql_store_result(_mysql);
            if(res == nullptr)
            {
                DLOG("mysql_store_result failed");
                return false;
            }
        }

        // 3. 获取结果集条数，得到结果集
        int row_num = mysql_num_rows(res);
        if(row_num == 0)
        {
            DLOG("the user information is not found!");
            return false;
        }
        else if(row_num != 1)
        {
            DLOG("the user information queried is not unique!!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);

        // 4. 将数据库中的用户信息填写到user对象中去，作为输出型参数
        user["id"] = (Json::UInt64)std::stol(row[0]);
        user["username"] = name;
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);

        // 5. 释放结果集
        mysql_free_result(res);
        return true;
    }

    // 通过id获取用户信息
    bool select_by_id(uint64_t id, Json::Value& user)
    {
#define SELECT_BY_ID "select username, score, total_count, win_count from user where id='%d';"
        // 1. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, SELECT_BY_ID, id);

        // 2. 执行语句，并且保存结果集到本地 -- 因为是查询语句，所以还是依然要加锁保证线程安全
        MYSQL_RES* res = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mtx); // 相当于加锁

            // 执行语句
            bool ret = mysql_util::mysql_exec(_mysql, query);
            if(ret == false)
            {
                DLOG("get user by id failed!!");
                return false;
            }

            // 保存结果集
            res = mysql_store_result(_mysql);
            if(res == nullptr)
            {
                DLOG("mysql_store_result failed");
                return false;
            }
        }

        // 3. 获取结果集条数，得到结果集
        int row_num = mysql_num_rows(res);
        if(row_num == 0)
        {
            DLOG("the user information is not found!");
            return false;
        }
        else if(row_num != 1)
        {
            DLOG("the user information queried is not unique!!");
            return false;
        }
        MYSQL_ROW row = mysql_fetch_row(res);

        // 4. 将数据库中的用户信息填写到user对象中去，作为输出型参数
        user["id"] = (Json::UInt64)id;
        user["username"] = row[0];
        user["score"] = (Json::UInt64)std::stol(row[1]);
        user["total_count"] = std::stoi(row[2]);
        user["win_count"] = std::stoi(row[3]);

        // 5. 释放结果集
        mysql_free_result(res);
        return true;
    }

    // 胜利处理函数 -- 胜利时天梯分数增加30分，战斗场次增加1，胜利场次增加1
    bool win(uint64_t id)
    {
#define WIN "update user set score=score+30, total_count=total_count+1, win_count=win_count+1 where id='%d';"
        // 1. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, WIN, id);

        // 2. 执行语句
        bool ret = mysql_util::mysql_exec(_mysql, query);
        if(ret == false)
        {
            DLOG("update win user info failed!!\n");
            return false;
        }
        return true;
    }

    // 失败处理函数 -- 失败时天梯分数减少30，战斗场次增加1，其他不变
    bool lose(uint64_t id)
    {
#define LOSE "update user set score=score-30, total_count=total_count+1 where id='%d';"
        // 1. 将语句格式化后放到query数组中
        char query[4096] = {0};
        sprintf(query, LOSE, id);

        // 2. 执行语句
        bool ret = mysql_util::mysql_exec(_mysql, query);
        if(ret == false)
        {
            DLOG("update lose user info failed!!\n");
            return false;
        }
        return true;
    }
};

#endif
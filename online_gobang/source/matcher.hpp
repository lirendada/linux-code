#ifndef __MY_MATCH_H__
#define __MY_MATCH_H__
#include "util.hpp"
#include "online.hpp"
#include "room.hpp"
#include "db.hpp"
#include <mutex>
#include <thread>
#include <condition_variable>
#include <list>

template <class T>
class match_queue
{
private:
    std::list<T> _block_queue;     // 阻塞队列 -- 用双向链表实现
    std::mutex _mtx;               // 互斥锁 -- 实现线程安全
    std::condition_variable _cond; // 条件变量 -- 主要用于阻塞消费者，当队列元素个数小于2的时候阻塞
public:
    // 获取队列元素个数
    int size()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        return _block_queue.size();
    }

    // 判断队列是否为空
    bool isEmpty()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        return _block_queue.empty();
    }

    // 阻塞线程
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _cond.wait(lock);
    }

    // 数据入队，并唤醒线程
    void push(T& data)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _block_queue.push_back(data);
        _cond.notify_all();
    }

    // 数据出队 -- 相当于匹配成功要进入房间，data是输出型参数
    bool pop(T& data)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        if(_block_queue.empty())
            return false;
        data = _block_queue.front();
        _block_queue.pop_front();
        return true;
    }

    // 移除指定的数据 -- 相当于取消匹配
    void remove(T& data)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _block_queue.remove(data);
    }
};


class match_manager
{
private:
    match_queue<uint64_t> _bronze; // 青铜段位队列
    match_queue<uint64_t> _silver; // 白银段位队列
    match_queue<uint64_t> _gold;   // 黄金段位队列
    std::thread _bronze_thread; // 青铜段位线程
    std::thread _silver_thread; // 白银段位线程
    std::thread _gold_thread;   // 黄金段位线程

    online_manager* _onlineptr; // 在线用户管理句柄
    user_table* _utableptr;     // 数据库用户表信息管理句柄
    room_manager* _roomptr;     // 房间管理句柄
public:
    match_manager(online_manager* onlineptr, user_table* utableptr, room_manager* roomptr)
        : _onlineptr(onlineptr), _utableptr(utableptr), _roomptr(roomptr),
          _bronze_thread(std::thread(&match_manager::_bronze_entry, this)),
          _silver_thread(std::thread(&match_manager::_silver_entry, this)),
          _gold_thread(std::thread(&match_manager::_gold_entry, this))
    { DLOG("匹配队列管理类初始化完毕...."); }

    // 根据玩家的天梯分数，来判定玩家档次，添加到不同的匹配队列
    bool addUser(uint64_t uid)
    {
        // 1. 根据用户ID，获取玩家信息
        Json::Value root;
        bool ret = _utableptr->select_by_id(uid, root);
        if(ret == false)
        {
            DLOG("获取玩家:%d 信息失败！！", uid);
            return false;
        }
        uint64_t score = root["score"].asUInt64();

        // 2. 添加到指定的队列中
        if(score < 2000)
            _bronze.push(uid);
        else if(score >= 2000 && score < 3000)
            _silver.push(uid);
        else
            _gold.push(uid);
        return true;
    }
    
    // 将用户从匹配队列中删除，也就是取消匹配
    bool delUser(uint64_t uid)
    {
        // 1. 根据用户ID，获取玩家信息
        Json::Value root;
        bool ret = _utableptr->select_by_id(uid, root);
        if(ret == false)
        {
            DLOG("获取玩家:%d 信息失败！！", uid);
            return false;
        }
        uint64_t score = root["score"].asUInt64();

        // 2. 将用户从匹配队列中删除
        if(score < 2000)
            _bronze.remove(uid);
        else if(score >= 2000 && score < 3000)
            _silver.remove(uid);
        else
            _gold.remove(uid);
        return true;
    }

private:
    // 三个段位各自的线程入口函数
    void _bronze_entry() { return thread_handle(_bronze); }
    void _silver_entry() { return thread_handle(_silver); }
    void _gold_entry() { return thread_handle(_gold); }

    // 总的处理线程入口函数细节的函数
    // 在这个函数中实现将用户到匹配队列、房间的分配、响应等操作
    void thread_handle(match_queue<uint64_t>& queue)
    {
        // 放到死循环中
        while(1)
        {
            // 1. 判断队列人数是否大于2，如果小于2则阻塞等待
            if(queue.size() < 2)
                queue.wait();

            // 2. 走到这代表人数够了，出队两个玩家
            //    这里有细节，如果第一个人出队的时候失败了，那么只需要continue重新开始出队
            //    但是如果是第二个人出队时候失败了，就要先将已经出队的第一个人的信息重新入队再continue
            uint64_t uid1;
            bool ret = queue.pop(uid1);
            if(ret == false)
                continue;
            
            uint64_t uid2;
            ret = queue.pop(uid2);
            if(ret == false)
            {
                queue.push(uid1); // 要先将出队的那个人重新放到队列中再continue
                continue;
            }

            // 3. 校验两个玩家是否在线，如果有人掉线，也就是通信句柄是无效的
            //    则要把另一个人重新添加入队列，因为当前玩家掉线，而另一个人则需要重新匹配
            wsserver_t::connection_ptr conn1 =  _onlineptr->get_conn_from_hall(uid1);
            if(conn1.get() == nullptr)
            {
                this->addUser(uid2);
                continue;
            }

            wsserver_t::connection_ptr conn2 =  _onlineptr->get_conn_from_hall(uid2);
            if(conn1.get() == nullptr)
            {
                this->addUser(uid1);
                continue;
            }

            // 4. 为两个玩家创建房间，并将玩家加入房间中 -- 创建失败的话要重新将用户放到匹配队列
            room_ptr rp = _roomptr->addRoom(uid1, uid2);
            if(rp.get() == nullptr)
            {
                this->addUser(uid1);
                this->addUser(uid2);
                continue;
            }

            // 5. 对两个玩家进行json数据响应
            Json::Value response;
            response["optype"] = "match_success";
            response["result"] = true;

            std::string body;
            json_util::serialize(response, body);
            conn1->send(body);
            conn2->send(body);
        }
    }
};

#endif
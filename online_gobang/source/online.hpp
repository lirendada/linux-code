#ifndef __MY_ONLINE_H__
#define __MY_ONLINE_H__
#include "util.hpp"
#include <mutex>
#include <unordered_map>

class online_manager
{
private:
    std::unordered_map<uint64_t, wsserver_t::connection_ptr> _hall_user; // 用于建立游戏大厅用户的ID与websocket通信连接的关系
    std::unordered_map<uint64_t, wsserver_t::connection_ptr> _room_user; // 用于建立游戏房间用户的ID与websocket通信连接的关系
    std::mutex _mtx; // 映射等操作需要加锁保护
public:
    // websocket连接建立的时候才会加入游戏大厅&游戏房间在线用户管理
    void enterHall(uint64_t uid, wsserver_t::connection_ptr& conn)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _hall_user[uid] = conn;
    }
    void enterRoom(uint64_t uid, wsserver_t::connection_ptr& conn)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _room_user[uid] = conn;
    }

    // websocket连接断开的时候，才会移除游戏大厅&游戏房间在线用户管理
    void exitHall(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _hall_user.erase(uid);
    }
    void exitRoom(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _room_user.erase(uid);
    }

    // 判断当前指定用户是否在游戏大厅/游戏房间
    bool isInHall(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto ret = _hall_user.find(uid);
        if(ret == _hall_user.end())
            return false;
        return true;
    }
    bool isInRoom(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto ret = _room_user.find(uid);
        if(ret == _room_user.end())
            return false;
        return true;
    }

    // 通过用户ID在游戏大厅/游戏房间用户管理中获取对应的websocket通信连接
    wsserver_t::connection_ptr get_conn_from_hall(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto ret = _hall_user.find(uid);
        if(ret == _hall_user.end())
            return wsserver_t::connection_ptr();
        return ret->second;
    }
    wsserver_t::connection_ptr get_conn_from_room(uint64_t uid)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        auto ret = _room_user.find(uid);
        if(ret == _room_user.end())
            return wsserver_t::connection_ptr();
        return ret->second;
    }
};

#endif
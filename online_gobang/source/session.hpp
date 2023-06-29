#ifndef __MY_SESSION_H__
#define __MY_SESSION_H__
#include "util.hpp"
#include <unordered_map>
#include <functional>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

typedef enum { UNLOGIN, LOGIN } STATUS;

class session
{
private:
    uint64_t _sessionID;          // 会话标识符
    uint64_t _userID;             // 用户ID
    STATUS _status;               // 用户状态
    wsserver_t::timer_ptr _timer; // 定时器
public:
    // 构造函数和析构函数
    session(uint64_t sessionID) : _sessionID(sessionID) 
    { 
        DLOG("SESSION %p 被创建！！", this); 
    }
    ~session() 
    { 
        DLOG("SESSION %p 被释放！！", this); 
    }

    // 判断状态接口
    bool isLogin() { return (_status == LOGIN); }

    // 获取成员变量接口
    uint64_t getSessionID() { return _sessionID; }
    uint64_t getUserID() { return _userID; }
    STATUS getStatus() { return _status; }
    wsserver_t::timer_ptr& getTimer() { return _timer; }

    // 给成员变量赋值接口
    void setUserID(uint64_t userID) { _userID = userID; }
    void setStatus(STATUS status) { _status = status; }
    void setTimer(const wsserver_t::timer_ptr& timer) { _timer = timer; }
};

#define SESSION_EXPIRE_TIME 30000 
#define SESSION_FOREVER -1         
using session_ptr = std::shared_ptr<session>; // 声明一个智能指针管理的会话对象类型

class session_manager
{
private:
    uint64_t _count;     // 会话ID分配计数器
    std::mutex _mtx;     // 互斥锁
    wsserver_t* _server; // websocketpp的服务器对象
    std::unordered_map<uint64_t, session_ptr> _hash; // 会话ID和会话信息的管理哈希表
public:
    // 构造函数和析构函数
    session_manager(wsserver_t* server)
        : _count(1), _server(server)
    {
        DLOG("session管理器初始化完毕！");
    }
    ~session_manager()
    {
        DLOG("session管理器即将销毁！");
    }

    // 创建session函数，返回一个智能指针管理的会话对象类型
    session_ptr add_session(uint64_t userID, STATUS status)
    {
        std::unique_lock<std::mutex> lock(_mtx); // 加锁保护

        // 1. 通过计数器创建一个session对象，由session_ptr管理
        session_ptr sp(new session(_count));     
        if(sp.get() == nullptr)
            return session_ptr();
        
        // 2. 设置会话状态，并且映射会话id和会话信息的关系
        sp->setStatus(status);
        sp->setUserID(userID);
        _hash[_count] = sp;

        // 3. 别忘了计数器要自增
        _count++; 
        return sp;
    }

    // 通过会话ID获取会话信息函数
    session_ptr get_session_by_sesssionID(uint64_t sessionID)
    {
        std::unique_lock<std::mutex> lock(_mtx); // 加锁保护

        auto ret = _hash.find(sessionID);
        if(ret == _hash.end())
            return session_ptr();
        return ret->second;
    }

    // 销毁session函数
    void removeSession(uint64_t sessionID)
    {
        std::unique_lock<std::mutex> lock(_mtx); // 加锁保护
        _hash.erase(sessionID);
    }

    // 设置session过期时间函数
    void set_session_expire_time(uint64_t sessionID, int ms)
    {
        // 通过websocketpp的定时器来完成session生命周期的管理。
        //      登录之后，创建session，这个session需要在指定时间、无通信后删除
        //      但是进入游戏大厅，或者游戏房间，这个session就应该永久存在，因为不可能说玩完游戏之后提示重新输入密码
        //      等到退出游戏大厅，或者游戏房间，这个session应该被重新设置为临时，在长时间无通信后被删除

        // 1. 创建session句柄
        session_ptr sp = get_session_by_sesssionID(sessionID);
        if(sp.get() == nullptr)
            return;

        // 2. 通过session句柄接口获取定时器
        wsserver_t::timer_ptr timer = sp->getTimer();

        // 3. 通过定时器和参数ms来分别处理四种不同情况
        //    - 其中定时器为空表示会话是永久的，因为没有设置；不为空则说明要定时删除会话
        //    - ms为SESSION_FOREVER代表要设置会话为永久；不为SESSION_FOREVER代表要设置过期时间为ms
        if(timer.get() == nullptr && ms == SESSION_FOREVER)
        {
            // 1. 在session永久存在的情况下，设置永久存在
            // 这种情况相当于不用设置，什么都不做
        }
        else if(timer.get() == nullptr && ms != SESSION_FOREVER)
        {
            // 2. 在session永久存在的情况下，设置指定时间之后被删除的定时任务
            timer = _server->set_timer(ms, std::bind(&session_manager::removeSession, this, sessionID));
            sp->setTimer(timer);
        }
        else if(timer.get() != nullptr && ms == SESSION_FOREVER)
        {
            // 3. 在session设置了定时删除的情况下，将session设置为永久存在

            // 首先就得将原来要删除的任务取消，但是就会触发对应的执行函数也就是删除会话，所以取消完要去重新添加会话信息
            timer->cancel();
            sp->setTimer(wsserver_t::timer_ptr()); // 并将该session的计数器更新一下，构造一个空的定时器表示为会话永久

            // 又因为该触发函数可能不会立马执行，所以我们不能马上就去重新添加上会话
            // 这里得再搞一个定时器，设置触发时间为0，触发函数为添加会话函数
            // 也就是此时添加会话函数的执行的顺序就排在了删除会话函数后，保证了不会提取删的情况！
            // 但是因为add_session函数是新建一个session，我们要添加的是原来这个session，所以创建一个子函数append来满足我们这个要求
            _server->set_timer(0, std::bind(&session_manager::append_session, this, sp));
        }
        else if(timer.get() != nullptr && ms != SESSION_FOREVER)
        {
            // 4. 在session设置了定时删除的情况下，将session重置删除时间。

            // 这种情况比较复杂，首先取消定时器，那么就会触发了触发函数去删除会话，我们就要重新添加会话
            // 因为触发函数并不是立马被执行的，为了保证添加会话一定成功，我们再设定一次定时器，时间设为0，触发函数是append添加会话函数
            // 这样子就能保证append添加会话函数在删除会话函数之后才执行！
            timer->cancel();
            sp->setTimer(wsserver_t::timer_ptr());
            _server->set_timer(0, std::bind(&session_manager::append_session, this, sp));
            
            // 然后再重新设置删除时间
            wsserver_t::timer_ptr tmp_tp = _server->set_timer(ms, std::bind(&session_manager::removeSession, this, sp->getSessionID()));
            sp->setTimer(tmp_tp); // 重新设置session关联的定时器
        }
    }
    
    void append_session(const session_ptr& sp)
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _hash[sp->getSessionID()] = sp;
    }
};

#endif
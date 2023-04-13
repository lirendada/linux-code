#pragma once
#include <iostream>
#include <pthread.h>

class Mutex
{
public:
    Mutex(pthread_mutex_t* mtx)
        :_mtx(mtx)
    {}
    void lock()
    {
        if(_mtx != nullptr)
            pthread_mutex_lock(_mtx);
    }
    void unlock()
    {
        if(_mtx != nullptr)
            pthread_mutex_unlock(_mtx);
    }
    ~Mutex()
    {}
private:
    pthread_mutex_t* _mtx; // 线程库中的锁对象指针
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* lock)
        :_guard(lock)
    {
        _guard.lock(); // 构造函数内加锁
    }
    ~LockGuard()
    {
        _guard.unlock(); // 析构函数内解锁
    }
private:
    Mutex _guard; // 封装的锁对象
};
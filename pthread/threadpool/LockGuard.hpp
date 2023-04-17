#pragma once
#include <iostream>
#include <pthread.h>

class Mutex
{
public:
    Mutex(pthread_mutex_t* pmutex = nullptr)
        : _pmutex(pmutex)
    {}
    void lock()
    {
        if(_pmutex != nullptr)
            pthread_mutex_lock(_pmutex);
    }
    void unlock()
    {
        if(_pmutex != nullptr)
            pthread_mutex_unlock(_pmutex);
    }
private:
    pthread_mutex_t* _pmutex;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t* pmutex)
        : _mutex(pmutex)
    {
        _mutex.lock();
    }
    ~LockGuard()
    {
        _mutex.unlock();
    }
private:
    Mutex _mutex;
};
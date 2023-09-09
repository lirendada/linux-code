#pragma once
#include <iostream>
#include <pthread.h>
#include "log.hpp"

class lockguard
{
private:
    pthread_mutex_t _mtx;
public:
    lockguard(pthread_mutex_t& mtx)
        : _mtx(mtx)
    {
        if(pthread_mutex_lock(&_mtx) != 0)
        {
            logMessage(Level::ERROR, "lock error");
            std::exception();
        }
    }

    ~lockguard()
    {
        if(pthread_mutex_unlock(&_mtx) != 0)
        {
            logMessage(Level::ERROR, "unlock error");
            std::exception();
        }
    }
};
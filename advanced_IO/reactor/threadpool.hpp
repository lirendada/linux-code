#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <exception>
#include <functional>
#include <pthread.h>
#include "log.hpp"
#include "lockguard.hpp"

static const int maxcapacity = 10; // 线程池默认最大数量

template <class T>
class threadpool
{
private:
    int _capacity;                  // 线程池最大容量
    std::vector<pthread_t> _tdpool; // 线程池数组
    std::queue<T> _workqueue;       // 任务队列
    pthread_mutex_t _mtx;           // 互斥锁，保护任务队列
    pthread_cond_t _cond;           // 条件变量，用于等待和唤醒线程
public:
    threadpool(int capacity = maxcapacity)
        : _capacity(capacity)
    {
        if(_capacity <= 0)
            std::exception();

        // 初始化互斥锁和条件变量
        if(pthread_mutex_init(&_mtx, nullptr) != 0)
            std::exception();

        if(pthread_cond_init(&_cond, nullptr) != 0)
        {
            pthread_mutex_destroy(&_mtx); // 记得释放已经申请的锁资源
            std::exception();
        }
        logMessage(Level::NORMAL, "互斥锁和条件变量初始化成功!");
        
        // 初始化线程，并且让线程脱离主线程
        for(int i = 0; i < _capacity; ++i)
        {
            logMessage(Level::NORMAL, "创建%d号线程", i);
            pthread_t pid = i;
            if(pthread_create(&pid, nullptr, thread_routine, this) != 0)
            {
                release();
                std::exception();
            }

            // 脱离主线程
            if(pthread_detach(pid) != 0)
            {
                release();
                std::exception();
            }
        }
        logMessage(Level::NORMAL, "线程池初始化完成");
    }

    ~threadpool()
    {
        pthread_mutex_destroy(&_mtx);
        pthread_cond_destroy(&_cond);
        release();
    }

    // 向任务队列中放入任务
    void put(const T& task)
    {
        std::lock_guard<pthread_mutex_t> lock(_mtx);
        _workqueue.push_back(task);
        pthread_cond_signal(&_cond); // 通知空闲线程拿任务
    }

    // 从任务队列中拿取任务
    T take()
    {
        T t = _workqueue.front();
        _workqueue.pop();
        return t;
    }
private:
    // 线程执行函数，负责拿出任务队列中的任务
    static void* thread_routine(void* arg)
    {
        threadpool<T>* tdp = static_cast< threadpool<T>* >(arg);
        while(true)
        {
            T t;
            {
                lockguard lock(tdp->_mtx);
                while(tdp->_workqueue.empty())
                {
                    pthread_cond_wait(&tdp->_cond, &tdp->_mtx);
                }
                t = tdp->take(); // 拿到任务
            }
            logMessage(Level::DEBUG, "线程拿到任务");

            // 接下来就是分配线程去处理该任务
        }
        return nullptr;
    }

    void release()
    {
        for(const auto& e : _tdpool)
            pthread_join(e, nullptr); // 回收线程
    }
};
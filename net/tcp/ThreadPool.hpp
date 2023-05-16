#pragma once 
#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <unistd.h>
#include "LockGuard.hpp"
#include "Thread.hpp"
#include "ThreadData.hpp"
#include "log.hpp"

using namespace ThreadNS;
const int MAXCAP = 10; // 线程池最大线程数量

template <class T>
class ThreadPool
{
public:
    // 启动所有线程的函数
    void run()
    {
        for(const auto& t : _threads)
        {
            // 使用ThreadData类装载线程池和名称，方便后面打印
            ThreadData<T>* td = new ThreadData<T>(this, t->threadname());
            t->start(handlerTask, td);  
            // std::cout << t->threadname() << " start......" << std::endl;
            logMessage(Level::DEBUG, "%s start......", t->threadname().c_str());
        }
    }

    // 向任务队列中放置任务的接口
    void put(const T& in)
    {
        // 队列的操作是线程不安全的，所以加锁
        LockGuard lock(&_mutex);
        _task_queue.push(in);
        pthread_cond_signal(&_cond); // 唤醒其中一个线程执行任务
    }

    // 拿取并且弹出任务队列中的任务
    T take()
    {
        // 此时只会有一个线程执行，所以不需要加锁
        T t = _task_queue.front();
        _task_queue.pop();
        return t;
    }

    ~ThreadPool()
    {
        pthread_cond_destroy(&_cond);
        pthread_mutex_destroy(&_mutex);
        for(const auto& t : _threads)
            delete t;
    }

    static ThreadPool<T>* GetInstance()
    {
        // 使用双层判断来减少加锁开销
        if(_ptp == nullptr)
        {
            pthread_mutex_lock(&_static_mutex);
            if(_ptp == nullptr)
            {
                _ptp = new ThreadPool<T>();
            }
            pthread_mutex_unlock(&_static_mutex);
        }
        return _ptp;
    }
private:
    // 封掉拷贝构造和赋值重载
    ThreadPool<T>& operator=(const ThreadPool<T>&) = delete;
    ThreadPool(const ThreadPool<T>&) = delete;

    // 构造函数设为私有
    ThreadPool(const int& maxcap = MAXCAP)
        :_cap(maxcap)
    {
        // 初始化工作
        pthread_cond_init(&_cond, nullptr);
        pthread_mutex_init(&_mutex, nullptr);
        for(int i = 0; i < _cap; ++i)
            _threads.push_back(new Thread());
    }
private:
    // 线程将来在此获取来自任务队列中的任务和执行任务
    static void* handlerTask(void* args)
    {
        ThreadData<T>* td = static_cast< ThreadData<T> *>(args);
        while(true)
        {
            T t;
            {
                LockGuard lock(&td->_threadpool->_mutex);
                while(td->_threadpool->_task_queue.empty())
                {
                    pthread_cond_wait(&td->_threadpool->_cond, &td->_threadpool->_mutex); // 阻塞直到被put了任务后被唤醒
                }
                t = td->_threadpool->take(); // 拿取、弹出任务队列中的任务
            }
            t();
        }
        delete td;
        return nullptr;
    }
private:
    int _cap;                      // 线程池容量
    std::vector<Thread*> _threads; // 线程等待容器
    std::queue<T> _task_queue;     // 任务队列
    pthread_cond_t _cond;          // 用来线程等待和唤醒线程的条件变量
    pthread_mutex_t _mutex;        // 互斥锁，保护共享资源--任务队列

    static pthread_mutex_t _static_mutex; // 静态互斥锁，来保护生成单例模式
    static ThreadPool<T>* _ptp;  // 静态对象指针，来生成单例对象
};

// 静态对象类外初始化
template <class T>
pthread_mutex_t ThreadPool<T>::_static_mutex;

template <class T>
ThreadPool<T>* ThreadPool<T>::_ptp = nullptr;
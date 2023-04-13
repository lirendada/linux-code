#pragma once
#include <iostream>
#include <queue>
#include <pthread.h>

static const int MAXCAP = 500;

template <class T>
class BlockQueue
{
public:
    BlockQueue(const int& maxcap = MAXCAP)
        :_maxcap(maxcap)
    {
        pthread_mutex_init(&_mutex, nullptr);
        pthread_cond_init(&_pcond, nullptr);
        pthread_cond_init(&_ccond, nullptr);
    }
    void put(const T& in) // 输入型参数：const&
    {
        pthread_mutex_lock(&_mutex);

        // 这里必须使用while，而不能是if
        while(is_full()) // 若队列为满则不能放数据
        {
            pthread_cond_wait(&_pcond, &_mutex);
        }
        // 走到这说明队列不为满，可以放资源
        _bq.push(in);
        pthread_cond_signal(&_ccond); // 唤醒消费者线程
        pthread_mutex_unlock(&_mutex);
    }
    void take(T* out) // 输出型参数：*   // 输入输出型参数：&
    {
        pthread_mutex_lock(&_mutex);

        // 这里必须使用while，而不能是if
        while(is_empty()) // 若队列为空不能取资源
        {
            pthread_cond_wait(&_ccond, &_mutex);
        }
        // 走到这说明队列不为空，可以取资源
        *out = _bq.front(); 
        _bq.pop();
        pthread_cond_signal(&_pcond); // 唤醒生产者线程
        pthread_mutex_unlock(&_mutex);
    }
    ~BlockQueue()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_pcond);
        pthread_cond_destroy(&_ccond);
    }
private:
    bool is_empty() const
    {
        return _bq.size() == 0;
    }

    bool is_full() const
    {
        return _bq.size() == _maxcap;
    }
private:
    std::queue<T> _bq;
    int _maxcap; // 队列中元素的上限
    pthread_mutex_t _mutex; // 互斥锁
    pthread_cond_t _pcond; // 生产者对应的条件变量
    pthread_cond_t _ccond; // 消费者对应的条件变量
};
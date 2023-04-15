#pragma once
#include <iostream>
#include <vector>
#include <semaphore.h>

static const int MAXSIZE =  5; // 环形队列最大数量

template <class T>
class RingQueue
{
public:
    RingQueue(const int& cap = MAXSIZE)
        :_cap(cap), _ringqueue(cap), _productor_index(0), _consumer_index(0)
    {
        // 信号量的初始化，其中互斥锁的信号量初始值为1
        sem_init(&_psem, 0, _cap);
        sem_init(&_csem, 0, 0);
        sem_init(&_pmutex, 0, 1);
        sem_init(&_cmutex, 0, 1);
    }
    ~RingQueue()
    {
        sem_destroy(&_psem);
        sem_destroy(&_csem);
        sem_destroy(&_pmutex);
        sem_destroy(&_cmutex);
    }
    void put(const T& in) // 输入型参数
    {
        // sem_wait(&_mutex); 在外面加锁的话，效率变低了

        sem_wait(&_psem);   // P操作
        sem_wait(&_pmutex); // 加锁，相当于pthread_mutex_lock();

        _ringqueue[_productor_index++] = in;
        _productor_index %= _cap; // 保证长度在队列长度内
         
        sem_post(&_pmutex); // 解锁，相当于pthread_mutex_unlock();
        sem_post(&_csem);   // V操作
    }
    void take(T* out) // 输出型参数
    {
        sem_wait(&_csem);   // P操作
        sem_wait(&_cmutex); // 加锁，相当于pthread_mutex_lock();

        *out = _ringqueue[_consumer_index++];
        _consumer_index %= _cap; // 保证长度在队列长度内

        sem_post(&_cmutex); // 解锁，相当于pthread_mutex_unlock();
        sem_post(&_psem);   // V操作
    }
private:
    std::vector<T> _ringqueue;  // 循环数组
    int _cap;                   // 队列容量
    int _productor_index;       // 生产者下标
    int _consumer_index;        // 消费者下标
    sem_t _psem;                // 生产者信号量--控制空闲空间数量
    sem_t _csem;                // 消费者信号量--控制已有数据数量
    sem_t _pmutex;              // 互斥锁信号量--防止多生产者竞争问题
    sem_t _cmutex;              // 互斥锁信号量--防止多消费者竞争问题
};
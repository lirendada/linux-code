#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <pthread.h>
#include <cstdio>

namespace ThreadNS
{
    class Thread
    {
        using func_t = std::function<void*(void*)>;
    public:
        Thread()
        {
            char namebuffer[1024];
            snprintf(namebuffer, sizeof namebuffer, "thread%d", _num++);
            _threadname = namebuffer;
        }

        // 创建线程
        void start(func_t callback, void *args = nullptr) 
        {
            _callback = callback;
            _args = args;
            pthread_create(&_t, nullptr, start_routine, this);
        }

        // 等待线程
        void join()
        {
            pthread_join(_t, nullptr);
        }

        std::string threadname()
        {
            return _threadname;
        }
    private:
        // 在类内创建线程，想让线程执行对应的方法，需要将方法设置成为static
        static void* start_routine(void* args) // 类内成员，有缺省参数！
        {
            Thread* _this = static_cast<Thread*>(args);
            return _this->_callback(_this->_args);
        }
    private:
        func_t _callback;        // 线程执行函数
        void* _args;             // 线程函数参数
        std::string _threadname; // 线程标识名称
        pthread_t _t;

        static int _num;         // 用于标识几号线程
    };

    int Thread::_num = 1;
}
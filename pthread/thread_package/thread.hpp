#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <pthread.h>
#include <assert.h>
using namespace std;

class Thread;

// 上下文类，利用组合的思想来访问Thread的成员变量和方法
class Context
{
public:
    Context(Thread* td, void* args)
        :_td(td), _args(args)
    {}
    ~Context()
    {}
public:
    Thread* _td;
    void* _args;
};

class Thread
{
    using func_t = function<void*(void*)>; // c++11包装器，包装线程执行的函数，并重命名为func_t
    const int NUM = 1024;
public:
    Thread(func_t func, void* args = nullptr, int number = 0)
        :_func(func), _args(args)
    {
        char namebuffer[NUM];
        snprintf(namebuffer, sizeof(namebuffer), "thread-%d", number);
        _name = namebuffer;

        // 意料之外用异常/if，意料之中用assert
        Context* ct = new Context(this, _args);
        int n = pthread_create(&_tid, nullptr, start_routine, ct);
        assert(n == 0);
        (void)n;
    }

    // 因为start_routine是系统接口，不能有this指针，所以必须得是static
    static void* start_routine(void* args)
    {
        // 问题是静态方法无法访问到类内成员变量和方法
        // 所以这里采用创建一个Context类，让Context对象来访问接口的方法
        // 此时args就是Context类的对象
        Context* ct = static_cast<Context*>(args);
        void* ret = ct->_td->run(ct->_args); // 让ct来访问run，此时就不会有因为系统调用不认识this指针的情况！

        delete ct;
        return ret;
    }

    void* run(void* args)
    {
        return _func(args);
    }

    void join()
    {
        int n = pthread_join(_tid, nullptr);
        assert(n == 0);
        (void)n;
    }

    ~Thread() 
    {}
private:
    string _name; // 线程名称
    func_t _func; // 线程执行的函数
    void* _args; // 传给线程执行函数的参数
    pthread_t _tid; // 用户态线程ID
};
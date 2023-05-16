#pragma once
#include <iostream>
#include <string>
#include "ThreadPool.hpp"

// 需要有ThreadPool的声明，不然会报错
template <class T>
class ThreadPool;

// 线程池与名称的封装，成员设为public给外部使用
template <class T>
class ThreadData
{
public:
    ThreadData(ThreadPool<T>* threadpool, const std::string& name)
        : _threadpool(threadpool), _name(name)
    {}
public:
    ThreadPool<T>* _threadpool;
    std::string _name;
};
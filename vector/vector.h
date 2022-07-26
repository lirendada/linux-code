#pragma once
#include<iostream>
#include<cstring>
#include<cassert>

using namespace std;

namespace liren
{
    template<class T>
    class vector
    {
    public:
        typedef T* iterator;
        typedef const T* const_iterator;
    public:
        vector()
            :_start(nullptr)
            ,_finish(nullptr)
             ,_end_of_storage(nullptr)
        {}


        size_t size()
        {
            return _finish - _start; 
        }
        size_t size() const
        {
            return _finish - _start;
        }

        size_t capacity()
        {
            return _end_of_storage - _start; 
        }
        size_t capacity() const
        {
            return _end_of_storage - _start;
        }

        iterator begin()
        {
            return _start;
        }
        const_iterator begin() const
        {
            return _start;
        }

        iterator end()
        {
            return _finish;
        }
        const_iterator end() const
        {
            return _finish;
        }

        T& operator[](size_t index)
        {
            assert(index < size());

            return _start[index];
        }
        const T& operator[](size_t index) const
        {
            assert(index < size());

            return _start[index];
        }

        void reserve(size_t n)
        {
            if(n > capacity())
            {
                size_t sz = size(); //防止下面的_finish赋值时候调用的size()出现错误，先保存下来
                T* tmp = new T[n];
                if(_start) //若一开始数组就为空则不用拷贝
                {
                    memcpy(tmp, _start, sz*sizeof(T));
                    
                    //for(size_t i = 0; i < sz; ++i)
                     //   tmp[i] = _start[i];

                    delete[] _start;
                }
            
                _start = tmp;
                _finish = _start + sz;
                _end_of_storage = _start + n;
            }   
        }

        void push_back(const T& x)
        {
            if(_finish == _end_of_storage)
            {
               size_t newcapacity = capacity() == 0 ? 4 : capacity() * 2;
               reserve(newcapacity);
            }

            *_finish = x;
            ++_finish;
        }






    private:
        iterator _start;//指向开头的指针
        iterator _finish;//指向最后一个元素的指针
        iterator _end_of_storage;//指向存储空间末尾的指针
    };

    void test1()
    {
        vector<int> v1;
        v1.push_back(1);
        v1.push_back(2);
        v1.push_back(3);
        v1.push_back(4);
        v1.push_back(5);

        vector<int>::iterator it = v1.begin();
        while(it != v1.end())
        {
            cout << *it << " ";
            it++;
        }
        cout << endl;
    }
}

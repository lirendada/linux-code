#include "thread.hpp"
#include <memory>
#include <unistd.h>

int ticket = 10;

class ThreadData
{
public:
    ThreadData(const string& name, pthread_mutex_t* mtx)
        :_name(name), _mtx(mtx)
    {}
    ~ThreadData()
    {}
public:
    string _name;
    pthread_mutex_t* _mtx;
};

void* thread_routine(void* args)
{
    ThreadData* td = static_cast<ThreadData*>(args);
    while(true)
    {
        pthread_mutex_lock(td->_mtx);
        if(ticket > 0)
        {
            usleep(12345); // 1秒=10^3毫秒=10^6微秒
            cout << "new thread -> name: " << td->_name << "，the ticket: " << ticket << endl;
            ticket--;

            pthread_mutex_unlock(td->_mtx); // 释放锁
        }
        else
        {
            pthread_mutex_unlock(td->_mtx); // 记得不满足也要释放锁，不然直接break会造成死锁
            break;
        }
        
        // 为了避免某个线程长时间占用CPU资源，在每次售票后通过usleep函数让线程睡眠一段时间
        // 其实这和我们生活上也是贴切的，买完票还需要执行其它工作，比如生成订单等等，所以抢完票之后需要等待一会
        usleep(1000);
    }
}

int main()
{
    pthread_mutex_t mtx; // 定义一把锁
    pthread_mutex_init(&mtx, nullptr); // 对锁进行初始化

    unique_ptr<Thread> t1(new Thread(thread_routine, new ThreadData("user1", &mtx), 1));
    unique_ptr<Thread> t2(new Thread(thread_routine, new ThreadData("user2", &mtx), 2));
    unique_ptr<Thread> t3(new Thread(thread_routine, new ThreadData("user3", &mtx), 3));
    unique_ptr<Thread> t4(new Thread(thread_routine, new ThreadData("user4", &mtx), 4));
    unique_ptr<Thread> t5(new Thread(thread_routine, new ThreadData("user5", &mtx), 5));
    t1->join();
    t2->join();
    t3->join();
    t4->join();
    t5->join();

    pthread_mutex_destroy(&mtx); // 释放锁
    return 0;
}
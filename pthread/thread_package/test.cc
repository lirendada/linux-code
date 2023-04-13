// #include "thread.hpp"
// #include "Mutex.hpp"
// #include <memory>
// #include <unistd.h>

// int ticket = 10;

// class ThreadData
// {
// public:
//     ThreadData(const string& name, pthread_mutex_t* mtx)
//         :_name(name), _mtx(mtx)
//     {}
//     ~ThreadData()
//     {}
// public:
//     string _name;
//     pthread_mutex_t* _mtx;
// };

// void* thread_routine(void* args)
// {
//     ThreadData* td = static_cast<ThreadData*>(args);
//     while(true)
//     {
//         // 为了让锁只对下面的抢票进行加锁，而不影响后面的抢票完的处理操作
//         // 这里用{}也就是代码块将下面加锁的区域括起来即可！
//         pthread_mutex_lock(td->_mtx);
//         pthread_mutex_lock(td->_mtx); // 造成死锁！
//         {
//             LockGuard lock(td->_mtx);
//             if(ticket > 0)
//             {
//                 usleep(12345); // 1秒=10^3毫秒=10^6微秒
//                 cout << "new thread -> name: " << td->_name << "，the ticket: " << ticket << endl;
//                 ticket--;

//                 // pthread_mutex_unlock(td->_mtx); // 释放锁
//             }
//             else
//             {
//                 // pthread_mutex_unlock(td->_mtx); // 记得不满足也要释放锁，不然直接break会造成死锁
//                 break;
//             }
//         }
        
//         // 为了避免某个线程长时间占用CPU资源，在每次售票后通过usleep函数让线程睡眠一段时间
//         // 其实这和我们生活上也是贴切的，买完票还需要执行其它工作，比如生成订单等等，所以抢完票之后需要等待一会
//         usleep(1000);
//     }
// }

// int main()
// {
//     pthread_mutex_t mtx; // 定义一把锁
//     pthread_mutex_init(&mtx, nullptr); // 对锁进行初始化

//     unique_ptr<Thread> t1(new Thread(thread_routine, new ThreadData("user1", &mtx), 1));
//     unique_ptr<Thread> t2(new Thread(thread_routine, new ThreadData("user2", &mtx), 2));
//     unique_ptr<Thread> t3(new Thread(thread_routine, new ThreadData("user3", &mtx), 3));
//     unique_ptr<Thread> t4(new Thread(thread_routine, new ThreadData("user4", &mtx), 4));
//     unique_ptr<Thread> t5(new Thread(thread_routine, new ThreadData("user5", &mtx), 5));
//     t1->join();
//     t2->join();
//     t3->join();
//     t4->join();
//     t5->join();

//     pthread_mutex_destroy(&mtx); // 释放锁
//     return 0;
// }

#include <iostream>
#include <pthread.h>
#include <unistd.h>
using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int count = 0;

void* odd_thread(void* args)
{
    const char* name = static_cast<const char*>(args);
    while(count < 20)
    {
        pthread_mutex_trylock(&mutex);
        while(count % 2 == 0) // 如果count为偶数则等待
        {
            pthread_cond_wait(&cond, &mutex); // 此时将锁解开，让偶数线程去执行
        }

        // 执行到这说明奇数线程被唤醒，打印完就唤醒偶数线程
        cout << name << " count is " << count++ << endl;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void* even_thread(void* args)
{
    const char* name = static_cast<const char*>(args);
    while(count < 20)
    {
        pthread_mutex_trylock(&mutex);
        while(count % 2 == 1) // 如果count为奇数则等待
        {
            pthread_cond_wait(&cond, &mutex); // 此时将锁解开，让奇数线程去执行
        }

        // 执行到这说明偶数线程被唤醒，打印完就唤醒奇数线程
        cout << name << " count is " << count++ << endl;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    // 下面用两个线程打印0~20
    // 奇数线程
    pthread_t odd;
    pthread_create(&odd, nullptr, odd_thread, (void*)"odd-thread");

    // 偶数线程
    pthread_t even;
    pthread_create(&even, nullptr, even_thread, (void*)"even_thread");

    // 记得最后等待新线程
    pthread_join(odd, nullptr);
    pthread_join(even, nullptr);
    return 0;
}
// #include "ThreadPool.hpp"
// #include "Thread.hpp"

// void* thread_routine(void* args)
// {
//     std::string meg = static_cast<const char*>(args);
//     while(true)
//     {
//         std::cout << "this is new thread" << meg << std::endl;
//     }
// }

// int main()
// {
//     Thread::Thread t1;
//     t1.start(thread_routine, (void*)"1");
//     Thread::Thread t2;
//     t2.start(thread_routine, (void*)"2");
//     t1.join();
//     t2.join();
//     return 0;
// }

// #include "ThreadPool.hpp"
// #include "Task.hpp"
// #include <memory>
// #include <ctime>
// #include <unistd.h>
// using namespace ThreadNS;
// int main()
// {
//     srand((unsigned int)time(nullptr));

//     std::unique_ptr<ThreadPool<CalTask>> tp(new ThreadPool<CalTask>());
//     tp->run();

//     while(true)
//     {
//         int x = rand() % 1000;
//         int y = rand() % 2000;
//         char op = oper[rand() % oper.size()];
//         CalTask t(x, y, op, caltask);
//         tp->put(t);
//         usleep(50000);
//     }
//     return 0;
// }


// #include "ThreadPool.hpp"
// #include "Task.hpp"
// #include <memory>
// #include <ctime>
// #include <unistd.h>
// using namespace ThreadNS;
// int main()
// {
//     srand((unsigned int)time(nullptr));

//     // 生成单例对象
//     ThreadPool<CalTask>* tp = ThreadPool<CalTask>::GetInstance();
//     tp->run();

//     // ThreadPool<CalTask>* tp2 = new ThreadPool<CalTask>(); ❌
//     // ThreadPool<CalTask> tp3; ❌

//     while(true)
//     {
//         int x = rand() % 1000;
//         int y = rand() % 2000;
//         char op = oper[rand() % oper.size()];
//         CalTask t(x, y, op, caltask);
//         tp->put(t);
//         usleep(50000);
//     }
//     return 0;
// }


#include <iostream>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock;

void *read_thread(void *arg) 
{
    while (true) 
    {
        pthread_rwlock_rdlock(&rwlock);
        std::cout << "Read thread " << arg << " get read lock." << std::endl;
        // 读取共享资源
        pthread_rwlock_unlock(&rwlock);
        std::cout << "Read thread " << arg << " release read lock." << std::endl;
        usleep(10000);
    }
}
void *write_thread(void *arg) 
{
    while (true) 
    {
        pthread_rwlock_wrlock(&rwlock);
        std::cout << "Write thread " << arg << " get write lock." << std::endl;
        // 写入共享资源
        pthread_rwlock_unlock(&rwlock);
        std::cout << "Write thread " << arg << " release write lock." << std::endl;
        usleep(10000);
    }
}
int main() 
{
    pthread_t read_tid[3], write_tid[2];
    pthread_rwlock_init(&rwlock, nullptr); // 初始化读写锁

    // 创建线程
    for (int i = 0; i < 3; i++)
        pthread_create(&read_tid[i], nullptr, read_thread, (void *)i);
    for (int i = 0; i < 2; i++)
        pthread_create(&write_tid[i], nullptr, write_thread, (void *)i);

    // 等待线程
    for (int i = 0; i < 3; i++)
        pthread_join(read_tid[i], nullptr);
    for (int i = 0; i < 2; i++) {
        pthread_join(write_tid[i], nullptr);
    }

    // 销毁读写锁
    pthread_rwlock_destroy(&rwlock);
    return 0;
}


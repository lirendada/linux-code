// #include <iostream>
// #include <unistd.h>
// #include <pthread.h>
// #include <semaphore.h>

// int global_value = 20;
// sem_t mutex; // 使用信号量来实现代替互斥锁

// void* thread_routine(void* args)
// {
//     char* namebuffer = static_cast<char*>(args);
//     while(global_value)
//     {
//         sem_wait(&mutex); // 将mutex减一，变成0，相当于加锁，其它线程会阻塞
//         if(global_value > 0)
//             std::cout << namebuffer << " the global_value is " << global_value-- << std::endl;
//         sem_post(&mutex); // 将mutex加一，变成1，相当于解锁，其它线程就能竞争该信号量

//         usleep(10000);
//     }
//     return nullptr;
// }

// int main()
// {
//     sem_init(&mutex, 0, 1); // 初始化互斥信号量为1，因为互斥锁只有0和1两种状态

//     // 创建多个线程
//     pthread_t threads[3];
//     for(int i = 0; i < 3; ++i)
//     {
//         char namebuffer[64];
//         snprintf(namebuffer, sizeof namebuffer, "new thread %d", i+1);
//         pthread_create(&threads[i], nullptr, thread_routine, namebuffer);
//     }

//     // 等待线程
//     for(int i = 0; i < 3; ++i)
//     {
//         pthread_join(threads[i], nullptr);
//     }

//     // 销毁信号量
//     sem_destroy(&mutex);
//     return 0;
// }


#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <ctime>
#include "RingQueue.hpp"
#include "Task.hpp"

template <class C, class S> // C:存储 S:保存
class RingQueues
{
public:
    RingQueues()
        : cal_rq(new RingQueue<C>)
        , save_rq(new RingQueue<S>)
    {}
    ~RingQueues()
    {
        delete cal_rq;
        delete save_rq;
    }
public:
    RingQueue<C>* cal_rq;
    RingQueue<S>* save_rq;
};

void* productor(void* args)
{
    RingQueue<CalTask>* cal_rq = (static_cast<RingQueues<CalTask, SaveTask>*>(args))->cal_rq;
    while(true)
    {
        // 生产任务
        int x = rand() % 1000;
        int y = rand() % 400;
        char op = oper[rand() % oper.size()];
        CalTask ct(x, y, op, caltask);

        // 放置任务
        cal_rq->put(ct);
        std::cout << pthread_self() << "->productor线程，任务放置完毕，任务为：" << ct.toTaskString() << std::endl;
        sleep(1);
    }
    return nullptr;
}

void* consumer(void* args)
{
    RingQueue<CalTask>* cal_rq = (static_cast<RingQueues<CalTask, SaveTask>*>(args))->cal_rq;
    RingQueue<SaveTask>* save_rq = (static_cast<RingQueues<CalTask, SaveTask>*>(args))->save_rq;
    while(true)
    {
        // 拿取任务
        CalTask ct;
        cal_rq->take(&ct);

        // 执行任务
        std::string result = ct();
        std::cout << pthread_self() << "->consumer线程，任务获取完毕，结果为：" << result << std::endl;

        // 向第二个环形队列放置执行结果
        SaveTask st(result, savetask);
        save_rq->put(st);
        std::cout << pthread_self() << "->consumer线程，任务放置...." << std::endl;
    }
    return nullptr;
}

void* saver(void* args)
{
    RingQueue<SaveTask>* save_rq = (static_cast<RingQueues<CalTask, SaveTask>*>(args))->save_rq;
    while(true)
    {
        // 拿取任务
        SaveTask st;
        save_rq->take(&st);

        // 执行任务/保存任务
        st();
        std::cout << pthread_self() << "->saver线程，任务保存完毕" << std::endl;
    }
    return nullptr;
}

int main()
{
    srand((unsigned int)time(nullptr) ^ getpid() ^ pthread_self()); // 种下随机种子

    RingQueues<CalTask, SaveTask> rqs; // 环形队列封装对象
    // 创建线程
    pthread_t p[5], c[3], s[3];
    for(int i = 0; i < 5; ++i)
        pthread_create(&p[i], nullptr, productor, &rqs);
    for(int i = 0; i < 3; ++i)
        pthread_create(&c[i], nullptr, consumer, &rqs);
    for(int i = 0; i < 3; ++i)
        pthread_create(&s[i], nullptr, saver, &rqs);

    // 等待线程
    for(int i = 0; i < 5; ++i)
        pthread_join(p[i], nullptr);
    for(int i = 0; i < 3; ++i)
    {
        pthread_join(c[i], nullptr);
        pthread_join(s[i], nullptr);
    }
    return 0;
}
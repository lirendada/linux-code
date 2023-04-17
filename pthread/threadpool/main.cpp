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

#include "ThreadPool.hpp"
#include "Task.hpp"
#include <memory>
#include <ctime>
#include <unistd.h>
using namespace ThreadNS;
int main()
{
    srand((unsigned int)time(nullptr));

    std::unique_ptr<ThreadPool<CalTask>> tp(new ThreadPool<CalTask>());
    tp->run();

    while(true)
    {
        int x = rand() % 1000;
        int y = rand() % 2000;
        char op = oper[rand() % oper.size()];
        CalTask t(x, y, op, caltask);
        tp->put(t);
        usleep(50000);
    }
    return 0;
}
#include "thread.hpp"
#include <memory>
#include <unistd.h>

void* thread_routine(void* args)
{
    string name = static_cast<const char*>(args);
    while(true)
    {
        cout << "i am new thread, name: " << name << endl;
        sleep(1);
    }
}

int main()
{
    unique_ptr<Thread> t1(new Thread(thread_routine, (void*)"hello linux", 1));
    unique_ptr<Thread> t2(new Thread(thread_routine, (void*)"hello c++", 2));
    unique_ptr<Thread> t3(new Thread(thread_routine, (void*)"hello java", 3));

    t1->join();
    t2->join();
    t3->join();
    return 0;
}
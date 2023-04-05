#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <cassert>
using namespace std;

// 新线程执行的函数
void* thread_routine(void* arg)
{
    char* mes = static_cast<char*>(arg);
    while(true)
    {
        cout << "LWP: " << syscall(SYS_gettid) << " 我是新线程，我正在运行：" << mes << endl;
        sleep(1);
    }
}

int main()
{
    pthread_t tid;
    // 创建新线程
    int n = pthread_create(&tid, nullptr, thread_routine, (void*)"new thread");
    assert(n == 0);
    static_cast<void>(n); // 防止realse下报错

    // 主线程执行
    while(true)
    {
        cout << "LWP: " << syscall(SYS_gettid) << " 我是主线程，我正在运行" << endl;
        sleep(2);
    }
    return 0;
}
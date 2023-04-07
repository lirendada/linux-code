// #include <iostream>
// #include <unistd.h>
// #include <pthread.h>
// #include <sys/syscall.h>
// #include <cassert>
// using namespace std;

// // 新线程执行的函数
// void* thread_routine(void* arg)
// {
//     char* mes = static_cast<char*>(arg);
//     while(true)
//     {
//         cout << "LWP: " << syscall(SYS_gettid) << " 我是新线程，我正在运行, my mes: " << mes << endl;
//         sleep(1);
//     }
// }

// int main()
// {
//     pthread_t tid;
//     // 创建新线程
//     int n = pthread_create(&tid, nullptr, thread_routine, (void*)"new thread");
//     assert(n == 0);
//     static_cast<void>(n); // 防止realse下报错

//     // 主线程执行
//     while(true)
//     {
//         cout << "LWP: " << syscall(SYS_gettid) << " 我是主线程，我正在运行" << endl;
//         sleep(2);
//     }
//     return 0;
// }


// #include <iostream>
// #include <unistd.h>
// #include <pthread.h>
// #include <vector>
// #include <string>
// #include <cstdio>
// using namespace std;

// struct ThreadData
// {
//     pthread_t _tid;
//     char _namebuffer[64];
// };

// // 新线程执行的函数
// void* thread_routine(void* arg)
// {
//     ThreadData* td = static_cast<ThreadData*>(arg);
//     int cnt = 10;
//     while(cnt)
//     {
//         // 顺便将cnt打印出来
//         cout << "新线程, name: " << td->_namebuffer << " cnt: " << cnt-- << " &cnt:" << &cnt << endl;
//         sleep(1);
//     }
//     delete td;
//     return nullptr;
// }
// int main()
// {
//     vector<ThreadData*> threads;
//     for(int i = 0; i < 10; ++i) // 创建10个线程
//     {
//         ThreadData* td = new ThreadData(); // 定义的是堆对象
//         snprintf(td->_namebuffer, sizeof td->_namebuffer, "%s, %d", "thread", i+1); // 格式化字符串
//         pthread_create(&td->_tid, nullptr, thread_routine, td->_namebuffer); // 创建新线程，将存放字符串的数组传过去
//     }

//     // 主线程执行
//     while(true)
//     {
//         cout << " 主线程running" << endl;
//         sleep(1);
//     }
//     return 0;
// }


// #include <iostream>
// #include <unistd.h>
// #include <pthread.h>
// #include <vector>
// #include <string>
// #include <cassert>
// #include <cstdio>
// using namespace std;

// struct ThreadData
// {
//     long long _i;
//     pthread_t _tid;
//     char _namebuffer[64];
// };

// void* thread_routine(void* arg)
// {
//     ThreadData* td = static_cast<ThreadData*>(arg);
//     int cnt = 4;
//     while(cnt)
//     {
//         cout << "新线程, name: " << td->_namebuffer << " cnt: " << cnt-- << endl;
//         sleep(1);
//     }

//     pthread_exit((void*)1314520); // 返回的退出信息设为td->_i，并且转为void*
//     // return (void*)td->_i; // 两种用法等价
// }
// int main()
// {
//     vector<ThreadData*> threads;
//     for(int i = 0; i < 4; ++i) // 创建4个线程
//     {
//         ThreadData* td = new ThreadData();
//         td->_i = i+1;
//         snprintf(td->_namebuffer, sizeof(td->_namebuffer), "%s%d", "thread", i+1);
//         pthread_create(&td->_tid, nullptr, thread_routine, td);
//         threads.push_back(td); // 记得尾插
//     }

//     // 线程等待
//     for(auto& e : threads)
//     {
//         void* ret = nullptr; // ret是个指针，用于下面函数传参，作为输出型参数
//         int n = pthread_join(e->_tid, &ret);
//         assert(n == 0);

//         // ret强转为long long，因为这里使用的是64位机器，指针为8字节，不能转为int类型
//         cout << "线程等待, name: " << e->_namebuffer << " 成功，退出码：" << (long long)ret << endl; 

//         // 释放线程空间
//         delete e;
//     }

//     while(true)
//     {
//         cout << " 主线程running" << endl;
//         sleep(1);
//     }
//     return 0;
// }


// #include <iostream>
// #include <unistd.h>
// #include <pthread.h>
// #include <cassert>
// #include <cstring>
// using namespace std;

// void* thread_routine(void* arg)
// {
//     string name = static_cast<const char*>(arg);
//     int cnt = 4;
//     while(cnt)
//     {
//         cout << "新线程, name: " << name << " cnt: " << cnt-- << endl;
//         sleep(1);
//     }

//     return nullptr;
// }

// int main()
// {
//     pthread_t td;
//     pthread_create(&td, nullptr, thread_routine, (void*)"new thread");
//     pthread_detach(td); // 将当前线程从主线程中分离
    
//     // 通过打印看看新线程是否被join了，是的话n是为0的
//     int n = pthread_join(td, nullptr);
//     cout << "result: " << n << " : " << strerror(n) << endl;

//     while(true)
//     {
//         cout << "主线程running" << endl;
//         sleep(1);
//     }
//     return 0;
// }


#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cassert>
#include <cstring>
using namespace std;

__thread int glo_val = 250;

void* thread_routine(void* arg)
{
    int cnt = 4;
    while(cnt)
    {
        cout << "新线程running, the glo_val: " << glo_val << " &glo_val: " << &glo_val << endl;
        glo_val++;
        sleep(1);
    }

    return nullptr;
}

int main()
{
    pthread_t td;
    pthread_create(&td, nullptr, thread_routine, (void*)"new thread");

    while(true)
    {
        cout << "主线程running, the glo_val: " << glo_val << " &glo_val: " << &glo_val << endl;
        sleep(1);
    }
    return 0;
}
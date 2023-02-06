// #include <iostream>
// #include <unistd.h>
// #include <cassert>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <cstdio>
// #include <cstring>
// #include <cstdlib>
// using namespace std;
// int main()
// {
//     // 1、创建管道文件，打开读写端
//     int fds[2];
//     int n = pipe(fds);
//     assert(n == 0);

//     // 2、创建子进程
//     pid_t id = fork();
//     assert(id >= 0);
//     if (id == 0)
//     {
//         // 子进程的通信代码
//         close(fds[0]); // 子进程关掉读端
//         int cnt = 0;
//         const char *msg = "i am child, sending msg now!"; // 子进程要发的信息
//         while (true)
//         {
//             char buffer[1024]; // 只有子进程能看到
//             // snprintf只是sprintf加上了写入个数
//             snprintf(buffer, sizeof(buffer), "child msg: %s[%d][%d]", msg, cnt++, getpid());
//             write(fds[1], buffer, strlen(buffer)); // 不需要算入'\0'
//             // sleep(2); // 每隔一秒写一次
//         }

//         close(fds[1]); // 建议最后关闭子进程写端
//         cout << "子进程关闭自己的写端" << endl;
//         exit(0);
//     }

//     // 父进程的通信代码
//     close(fds[1]); // 父进程关掉写端
//     while (true)
//     {
//         sleep(2);
//         char buffer[1024];
//         //  cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << endl;
//         ssize_t n = read(fds[0], buffer, sizeof(buffer) - 1); // 多留一个位置放\0
//         // cout << "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" << endl;
//         if (n > 0)
//         {
//             buffer[n] = '\0';
//             cout << "Get msg# " << buffer << " parent_pid: " << getpid() << endl;
//         }
//         else if(n == 0)
//         {
//             cout << "read: " << n << endl;
//             break;
//         }

//         break; // 直接break
//     }
//     close(fds[0]); // 提前关闭读端
//     cout << "父进程进程关闭自己的读端" << endl;

//     // 回收子进程
//     int status = 0;
//     n = waitpid(id, &status, 0);
//     assert(n == id);
//     cout << "pid-> " << n << " 终止信号：" << (status & 0x7F) << endl;

//     return 0;
// }

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <unistd.h>

using namespace std;

// 父进程进行读取，子进程进行写入
int main()
{
    // 第一步：创建管道文件，打开读写端
    int fds[2];
    int n = pipe(fds);
    assert(n == 0);

    // 第二步: fork
    pid_t id = fork();
    assert(id >= 0);
    if (id == 0)
    {
        // 子进程进行写入
        close(fds[0]);
        // 子进程的通信代码
        // string msg = "hello , i am child";
        const char *s = "我是子进程,我正在给你发消息";
        int cnt = 0;
        while (true)
        {
            cnt++;
            char buffer[1024]; // 只有子进程能看到！
            snprintf(buffer, sizeof buffer, "child->parent say: %s[%d][%d]", s, cnt, getpid());
            // 写端写满的时候，在写会阻塞，等对方进行读取!
            write(fds[1], buffer, strlen(buffer));
            //cout << "count: " << cnt << endl;
            // sleep(50); //细节，我每隔1s写一次
            // break;
        }

        // 子进程
        close(fds[1]); // 子进程关闭写端fd
        cout << "子进程关闭自己的写端" << endl;
        // sleep(10000);
        exit(0);
    }
    // 父进程进行读取
    close(fds[1]);
    // 父进程的通信代码
    while (true)
    {
        sleep(2);
        char buffer[1024];
        // cout << "AAAAAAAAAAAAAAAAAAAAAA" << endl;
        // 如果管道中没有了数据，读端在读，默认会直接阻塞当前正在读取的进程！
        ssize_t s = read(fds[0], buffer, sizeof(buffer) - 1);
        // cout << "BBBBBBBBBBBBBBBBBBBBBB" << endl;
        if (s > 0)
        {
            buffer[s] = 0;
            cout << "Get Message# " << buffer << " | my pid: " << getpid() << endl;
        }
        else if(s == 0)
        {
            //读到文件结尾
            cout << "read: " << s << endl;
            break;
        }
        break;

        // 细节：父进程可没有进行sleep
        // sleep(5);
    }
    close(fds[0]);
    cout << "父进程关闭读端" << endl;

    int status = 0;
    n = waitpid(id, &status, 0);
    assert(n == id);

    cout <<"pid->"<< n << " : "<< (status & 0x7F) << endl;


    // 0,1,2-> 3,4
    // 谁是读取，谁是写入
    // [0]: 读取，嘴巴，读书的
    // [1]: 写入，钢笔，写的
    // cout << "fds[0]: " << fds[0] << endl;
    // cout << "fds[1]: " << fds[1] << endl;
    return 0;
}
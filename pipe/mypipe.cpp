#include <iostream>
#include <unistd.h>
#include <cassert>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;
int main()
{
    // 1、创建管道文件，打开读写端
    int fds[2];
    int n = pipe(fds);
    assert(n == 0);

    // 2、创建子进程
    pid_t id = fork();
    assert(id >= 0);
    if (id == 0)
    {
        // 子进程的通信代码
        close(fds[0]); // 子进程关掉读端
        int cnt = 0;
        const char *msg = "i am child, sending msg now!"; // 子进程要发的信息
        while (true)
        {
            char buffer[1024]; // 只有子进程能看到
            // snprintf只是sprintf加上了写入个数
            snprintf(buffer, sizeof(buffer), "child msg: %s[%d][%d]", msg, cnt++, getpid());
            write(fds[1], buffer, strlen(buffer)); // 不需要算入'\0'
            // sleep(2); // 每隔一秒写一次
        }

        close(fds[1]); // 建议最后关闭子进程写端
        cout << "子进程关闭自己的写端" << endl;
        exit(0);
    }

    // 父进程的通信代码
    close(fds[1]); // 父进程关掉写端
    while (true)
    {
        sleep(1);
        char buffer[1024];
        //  cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << endl;
        ssize_t n = read(fds[0], buffer, sizeof(buffer) - 1); // 多留一个位置放\0
        // cout << "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" << endl;
        if (n > 0)
        {
            buffer[n] = '\0';
            cout << "Get msg# " << buffer << " parent_pid: " << getpid() << endl;
        }
        else if(n == 0)
        {
            cout << "read: " << n << endl;
            break;
        }

        break; // 直接break
    }
    close(fds[0]); // 提前关闭读端
    cout << "父进程进程关闭自己的读端" << endl;

    // 回收子进程
    int status = 0;
    n = waitpid(id, &status, 0);
    assert(n == id);
    cout << "pid-> " << n << " 终止信号：" << (status & 0x7F) << endl;

    return 0;
}
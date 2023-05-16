#pragma once
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* DEV = "/dev/null";

void daemonSelf(const char* curPath = nullptr)
{
    // 1.让调用进程忽略掉异常的信号
    // 比如这里的管道信号，防止如果没有客户端读取的时候，服务端如果在写，管道会异常退出，所以我们将其忽略掉
    signal(SIGPIPE, SIG_IGN);

    // 2.调用setsid()创建一个自成会话，并且调用setsid()的进程不是组长
    // 此时就要创建一个子进程，然后将当前进程给退出，让子进程去调用即可
    if(fork() > 0) exit(0); // 当前进程退出
    pid_t n = setsid();
    assert(n != 1);

    // 3.守护进程需要脱离终端，需要关闭或者重定向以前进程默认打开的文件
    // 很显然关闭那些文件是不太好的做法，所以优先选择重定向
    // 而我们可以重定向到/dev/null这个文件中，相当于linux中的一个垃圾桶
    // 如果创建这个重定向文件失败了，我们才采用关闭文件的方式
    int fd = open(DEV, O_RDWR); 
    if(fd != -1)
    {
        // 重定向到/dev/null中
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
    }
    else
    {
        close(0);
        close(1);
        close(2);
    }

    // 4. 可加的选项，进程执行路径发生更改
    if(curPath != nullptr)
        chdir(curPath);
}
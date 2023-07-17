#pragma once
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* DEV = "/dev/null";

void daemonSelf(const char* curPath = nullptr)
{
    // 1. 创建子进程并退出父进程
    // 此时就要创建一个子进程，然后将当前进程给退出，让子进程去调用即可
    pid_t pid = fork();
    if(pid > 0) 
    {
        exit(0); // 退出当前父进程
    }
    else if(pid < 0)
    {
        perror("fork error!");
        exit(1);
    }

    // 2. 子进程调用setsid()创建一个自成会话，调用setsid()的进程不能是组长，组长是之前的父进程！
    if(setsid() < 0) 
    {
        perror("setsid error");
        exit(1);
    }

    // 3. 让调用进程忽略掉容易异常的信号
    // 忽略SIGHUP信号的目的是为了防止守护进程在终端断开时终止
    // 忽略SIGPIPE信号是防止当进程向一个已经关闭写端的管道写入数据时，内核会向进程发送SIGPIPE信号，或者当进程向一个已经关闭的socket连接写入数据时，内核也会向进程发送SIGPIPE信号。
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SIG_IGN);

    // 4. 创建孙子进程后再次fork，以防止守护进程重新打开控制终端
    pid = fork();
    if(pid > 0) 
    {
        exit(0); // 退出当前父进程
    }
    else if(pid < 0)
    {
        perror("fork error!");
        exit(1);
    }

    // 5. 守护进程需要脱离终端，需要关闭或者重定向以前进程默认打开的文件
    // 很显然关闭那些文件是不太好的做法，所以优先选择重定向
    // 而我们可以重定向到 ‘/dev/null’ 这个文件中，相当于linux中的一个垃圾桶
    // 如果创建这个重定向文件失败了，我们才采用关闭文件的方式
    int fd = open(DEV, O_RDWR); 
    if(fd != -1)
    {
        // 打开文件成功，则重定向到/dev/null中
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
    }
    else
    {
        // 失败则直接关闭
        close(0);
        close(1);
        close(2);
    }

    // 6. 可选项：将进程执行路径更改
    if(curPath != nullptr)
        chdir(curPath);
}
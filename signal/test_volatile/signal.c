// #include <stdio.h>
// #include <signal.h>

// volatile int quit = 1; // 定义一个全局变量

// void handler(int signo)
// {
//     printf("%d号信号，正在被捕捉!\n", signo);
//     printf("quit: %d", quit);
//     quit = 0; // 将全局变量改为0
//     printf("-> %d\n", quit);
// }

// int main()
// {
//     signal(2, handler);
//     while(quit == 1); // 若quit为1则一直循环
//     printf("main正常退出!\n");
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
void handler(int signo)
{
    pid_t id;
    while((id = waitpid(-1, NULL, WNOHANG)) > 0) // 使用WNOHANG才能保证非阻塞查询
    {
        printf("wait child success: %d\n", id);
    }
    printf("child is quit! %d\n", getpid());
}
int main()
{
    signal(SIGCHLD, SIG_IGN); // 直接通过SIG_IGN忽略，即可回收僵尸子进程
    pid_t cid;
    if((cid = fork()) == 0)
    {  
        // child
        printf("child : %d\n", getpid());
        sleep(3);
        exit(1);
    }

    // father
    while(1)
    {
        printf("father proc is doing some thing!\n");
        sleep(1);
    }
    return 0;
}
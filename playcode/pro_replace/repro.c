#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t id = fork();
    if(id == 0)
    {
        // child
        printf("我是子进程，pid：%d，ppid：%d\n", getpid(), getppid());

        putenv("MYENV=helloLinux"); // 将自定义环境变量添加到系统中去
        extern char** environ; // 声明一下防止报错
        execle("./mycmd", "mycmd", NULL, environ); // 将系统中的环境变量传过去
        
        exit(1);
    }

    // father
    int status = 0;
    pid_t ret = waitpid(id, &status, 0);
    if(ret > 0)
    {
        printf("child status -> sig: %d, code: %d\n", status & 0x7F, (status >> 8) & 0xFF);
    }
    else
    {
        printf("wait error!\n");
    }
    return 0;
}

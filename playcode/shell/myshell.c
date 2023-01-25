#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>

#define NUM 1024  // 定义用户输入的最大数，其中个数为1023个，最后一位给'\0'
#define OPTION_NUM 64 // 定义切割后每个选项的最大个数
char lineCommand[NUM];  // 存放用户输入的字符串数组
char* myargv[OPTION_NUM]; // 存放切割后单词的指针数组

int lastSig = 0;
int lastCode = 0;

int main()
{
    // 因为shell是循环输入的，所以要套在死循环里面
    while(1)
    {
        // 获取命令行
        printf("[%s@%s %s]~ ", getenv("USER"), getenv("HOSTNAME"), getenv("PWD"));
        fflush(stdout);
    
        // 获取用户输入
        char* s = fgets(lineCommand, sizeof(lineCommand) - 1, stdin);
        assert(s != NULL); // 检测一下输入是否正确
        lineCommand[strlen(lineCommand) - 1] = '\0'; // 由于我们按回车会有一个'\n'，我们将其设为'\0'
        (void)s; // 这个操作是防止release的时候报错

        // 切割字符串
        myargv[0] = strtok(lineCommand, " ");
        int i = 1;

        // 若是ls，可以给它加上颜色
        if(myargv[0] != NULL && strcmp("ls", myargv[0]) == 0)
            myargv[i++] = (char*)"--color=auto";

        // 没有子串后，strtok最后切割完返回的是NULL，用myargv[i]接收判断后退出
        while(myargv[i++] = strtok(NULL, " "))
        {}
    
        // 若是cd，则可以直接在当前进程修改
        // 像这种指令不需要子进程来执行，而是让shell自己执行的命令，叫做内建/内置命令
        if(myargv[0] != NULL & strcmp("cd", myargv[0]) == 0)
        {
            if(myargv[1] != NULL)
                chdir(myargv[1]);
            continue;
        }

        // echo $? 时候打印出退出码和终止信号
        if(myargv[0] && myargv[1] && strcmp(myargv[0], "echo") == 0)
        {
            if(strcmp(myargv[1], "$?") == 0)
                printf("退出状态：%d，终止信号：%d\n", lastCode, lastSig);
            else
                printf("%s\n", myargv[1]);
            continue;
        }

        // 用条件编译，测试是否成功
    #ifdef DEBUG
        for(int i = 0; myargv[i]; ++i)
            printf("myargv[%d]：%s\n", i, myargv[i]);
    #endif

        // 执行命令
        pid_t id = fork();
        assert(id != -1);

        if(id == 0)
        {
            // 由于指令输入习惯，所以选择最适合的execvp
            execvp(myargv[0], myargv);

            // 若失败终止信号设为1
            exit(1);
        }

        int status = 0;
        pid_t ret = waitpid(id, &status, 0); // 回收已退出的子进程的信息
        assert(ret > 0);
        (void)ret; // 防止release时候报错

        // 将最近一次的退出码和终止信号更新
        lastCode = (status >> 8) & 0xFF;
        lastSig = status & 0x7F;
    }
    return 0;
}

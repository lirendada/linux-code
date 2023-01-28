#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define NUM 1024  // 定义用户输入的最大数，其中个数为1023个，最后一位给'\0'
#define OPTION_NUM 64 // 定义切割后每个选项的最大个数
char lineCommand[NUM];  // 存放用户输入的字符串数组
char* myargv[OPTION_NUM]; // 存放切割后单词的指针数组

int lastSig = 0;
int lastCode = 0;

#define NONE_REDIR 0 // 非重定向
#define INPUT_REDIR 1 // 输入重定向
#define OUTPUT_REDIR 2 // 输出重定向
#define APPEND_REDIR 3 // 追加重定向
int redirType = NONE_REDIR; // 重定向类型默认为非重定向
char* redirFile = NULL; // 重定向的文件，默认为空

// 检测是否重定向和分割函数
void commandCheck(char* commands)
{
    assert(commands);

    char* start = commands;
    char* end = commands + strlen(commands);
    while(start < end)
    {
        if(*start == '>')
        {
            // 比如"ls -a -l >  file.txt"或者"ls -a -l >>  file.txt"
            *start = '\0';
            start++;
            if(*start == '>') // 判断是否为追加重定向
            {
                redirType = APPEND_REDIR;
                start++;
            }
            else
            {
                redirType = OUTPUT_REDIR;
            }

            while(*start == ' ') // 跳过多余空格
                start++;

            // 填写重定向信息
            redirFile = start;
            break;
        }
        else if(*start == '<')
        {
            // 比如"cat < file.txt"
            *start = '\0';
            ++start;
            while(*start == ' ') // 跳过多余空格
                start++;

            // 填写重定向信息
            redirType = INPUT_REDIR;
            redirFile = start;
            break;
        }
        else
        {
            start++;
        }
    }
}

int main()
{
    // 因为shell是循环输入的，所以要套在死循环里面
    while(1)
    {
        // 记得更新一下重定向状态
        redirFile = NULL;
        redirType = 0;
        errno = 0;

        // 获取命令行
        printf("[%s@%s %s]~ ", getenv("USER"), getenv("HOSTNAME"), getenv("PWD"));
        fflush(stdout);
    
        // 获取用户输入
        char* s = fgets(lineCommand, sizeof(lineCommand) - 1, stdin);
        assert(s != NULL); // 检测一下输入是否正确
        lineCommand[strlen(lineCommand) - 1] = '\0'; // 由于我们按回车会有一个'\n'，我们将其设为'\0'
        (void)s; // 这个操作是防止release的时候报错

        // 比如"ls -a -l > file.txt"分割为"ls -a -l"和"file.txt"
        // 只需要找到重定向符号，将它们置为'\0'即可！
        commandCheck(lineCommand);

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
            // 因为命令是子进程执行的，真正重定向的工作一定是子进程来完成的
            // 如何重定向，是父进程要给子进程提供信息的
            // 又因为进程独立性，子进程重定向后其files_struct是不会影响父进程的
            // 并且因为被打开文件是共享的，所以就能达到子进程重定向，父进程不受影响且看到效果的目的
            if(redirType == NONE_REDIR)
            {
                // 什么都不做
            }
            else if(redirType == INPUT_REDIR)
            {
                int fd = open(redirFile, O_RDONLY);
                if(fd != -1)
                    dup2(fd, stdin->_fileno);
                else
                {
                    perror("open");
                    exit(errno);
                }
            }
            else if(redirType == OUTPUT_REDIR)
            {
                int fd = open(redirFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(fd < 0)
                {
                    perror("open");
                    exit(errno);
                }
                dup2(fd, stdout->_fileno);
            }
            else if(redirType == APPEND_REDIR)
            {
                int fd = open(redirFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                if(fd < 0)
                {
                    perror("open");
                    exit(errno);
                }
                dup2(fd, stdout->_fileno);
            }

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

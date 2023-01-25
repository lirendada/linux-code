#include <stdio.h>
#include <unistd.h>
int grobal_val = 10; // 全局变量
int main()
{
    pid_t id = fork();
    if(id == 0)
    {
        int cnt=0;
        while(1)
        {
            printf("子进程:pid = %d, ppid = %d | grobal_val = %d, &grobal_val = %p\n",getpid(), getppid(), grobal_val, &grobal_val);
            sleep(1);
            ++cnt;
            if(cnt==5)
            {
                grobal_val=300;
                printf("子进程已更改全局变量grobal_val\n");
            }
        }
    }
    else if(id > 0)
    {
        while(1)
        {
            printf("父进程:pid = %d, ppid = %d | grobal_val = %d, &grobal_val = %p\n",getpid(), getppid(), grobal_val, &grobal_val);
            sleep(1);
        }
    }
    else 
    {
        printf("fork error\n");
        return 1;
    }
    return 0;
}

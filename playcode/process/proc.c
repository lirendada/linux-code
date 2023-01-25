#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//int main()
//{
//    pid_t id = fork();
//    if(id == 0)
//    {
//        // 子进程
//        int cnt = 3;
//        while(cnt)
//        {
//            printf("子进程：%d 父进程：%d  cnt = %d\n", getpid(), getppid(), cnt--);
//            sleep(1);
//        }
//     //  exit(12);
//        // 野指针，异常终止
//        int* ptr = NULL;
//       *ptr = 10;
//        exit(0);
//    }
//
//    int status = 0;
//    pid_t ret = waitpid(id, &status, 0);
//    if(id > 0)
//    {
//        // 父进程
//        if(ret != -1)
//        {
//            if(WIFEXITED(status))
//                printf("wait success, 退出状态：%d\n", WEXITSTATUS(status));
//            else
//                printf("quit error, 终止信号：%d\n", WTERMSIG(status));
//        }
//    }
//    return 0;
//}
//int main()
//{
//    pid_t id = fork();
//    if(id == 0)
//    {
//        // 子进程
//        int cnt = 3;
//        while(cnt)
//        {
//            printf("我是子进程：%d, 父进程：%d, cnt：%d\n", getpid(), getppid(), cnt);
//            cnt--;
//            sleep(1);
//        }
//        exit(0); // 子进程退出
//    }
//
//    // 父进程
//    sleep(5);
//    pid_t ret = wait(NULL);
//    if(id > 0)
//        printf("wait success：%d\n", ret);
//    sleep(2);
//
//    return 0;
//}

//int main()
//{
//    printf("running ...");
//    sleep(3);
//    // 比较两者的区别
//   // exit(1);
//     exit(1);
//    
//    printf("done ...");
//    return 0;
//}
//int main()
//{
//    for(int i = 0; i < 134; ++i)
//    {
//        printf("num[%d]:%s\n", i, strerror(i));
//    }
//    return 0;
//}
//int main()
//{
//    int cnt = 0;
//    while(1)
//    {
//        int ret = fork();
//        if(ret < 0)
//        {
//            printf("fork error!, cnt: %d\n", cnt);
//            break;
//        }
//        else if(ret == 0)
//        {
//            // 子进程不断循环
//            while(1) 
//            {
//                printf("子进程:pid = %d,ppid = %d\n",getpid(), getppid());
//                sleep(1);
//            }
//        }
//        // 父进程不断循环不断产生子进程
//        cnt++;
//    }
//    return 0;
//}    
//int main()    
//{    
//    pid_t pid;    
//    printf("Before: pid is %d\n", getpid());    
//    if((pid = fork()) == -1)
//    {
//        perror("fork()");
//        exit(1);
//    }
//    printf("After:pid is %d, fork return %d\n", getpid(), pid);    
//    sleep(1);    
//    return 0;    
//}    
//int grobal_val = 100;
#include <assert.h>
 
#define NUM 5
typedef void (*func_t)(); //函数指针
func_t handlerTask[NUM];//函数指针数组
 
//任务
void task1()
{
    printf("任务1\n");
}
void task2()
{
    printf("任务2\n");
}
void task3()
{
    printf("任务3\n");
}
 
void loadTask()
{
    memset(handlerTask, 0, sizeof(handlerTask));//将函数指针数组初始化为0
    handlerTask[0] = task1;//函数指针数组handlerTask[0]存放task1的地址
    handlerTask[1] = task2;
    handlerTask[2] = task3;
}
int main()
{
    pid_t id = fork();
    assert(id != -1);
    if(id == 0)
    {
        //child
        int cnt = 3;
        while(cnt)
        {
            printf("child running, pid: %d, ppid: %d, cnt: %d\n", getpid(), getppid(), cnt--);
            sleep(1);
        }
        exit(10);
    }
    loadTask();//加载任务

    // parent
    int status = 0;
    while(1)//父进程对子进程状态轮询
    {
        pid_t ret = waitpid(id, &status, WNOHANG); // 第三个参数为0表示阻塞式等待，为WNOHANG表示非阻塞式等待
        if(ret == 0) // 等于0代表没有被等待的进程暂未退出
        {
            printf("wait done, but child is running...., parent running other things\n");
            for(int i = 0; handlerTask[i] != NULL; i++) // 遍历到NULL，即0停止
            {
                handlerTask[i](); // 采用回调的方式，执行我们想让父进程在空闲的时候做的事情
            }
        }
        else if(ret > 0) // waitpid调用成功，并且子进程退出，返回值为被等待子进程的pid
        {
            printf("wait success, exit code: %d, sig: %d\n", (status>>8)&0xFF, status & 0x7F);
            break;
        }
        else // 等于-1表示等待失败，waitpid中的第一个参数值传错会导致调用失败
        {
            printf("waitpid call failed\n");
            break;
        }
        sleep(1);
    }
    return 0;
}



//int main()
//{
//    pid_t id = fork();
//    if(id == 0)
//    {
//        printf("子进程:pid = %d,ppid = %d | grobal_val = %d, &grobal_val = %p\n",getpid(), getppid(), grobal_val, &grobal_val);
//    }
//    else if(id > 0)
//    {
//        printf("父进程:pid = %d,ppid = %d | grobal_val = %d, &grobal_val = %p\n", getpid(), getppid(), grobal_val, &grobal_val);
//        sleep(1);
//    }
//    else 
//    {
//        printf("fork error\n");
//        return 1;
//    }
//    return 0;
//}

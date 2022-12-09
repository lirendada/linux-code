#include <stdio.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <stdlib.h>
int main()  
{
    //pid_t id = fork();

    //if(id == 0)
    //{
    //  	while(1)
    //    {
    //        printf("子进程,我的ID是:%d, 父进程PID是:%d, ID是:%d\n", getpid(),getppid(),id); 
    //        sleep(1);
    //    }
    //}
    //else if(id > 0)
    //{
    //    while(1)
    //    {
    //        printf("父进程,我的ID是:%d, 父进程PID是:%d, ID是:%d\n", getpid(),getppid(),id); 
    //    	sleep(1);
    //    }
    //}
    
    int id = fork();    
    if(id == 0)    
    {    
        while(1)
        {
        	printf("child, pid=%d, ppid=%d\n", getpid(), getppid());    
        	sleep(5);
        }
    }    
    else    
    {    
        printf("parent, pid=%d, ppid=%d\n", getpid(), getppid());           
        sleep(2);    
        exit(1); 
    }    
    return 0;                                               
}

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <signal.h>
using namespace std;

void count(int n)
{
    while(n >= 0)
    {
        printf("second : %2d\r", n--);
        fflush(stdout);
        sleep(1);
    }
    cout << endl;
}

void myhandler(int signo)
{
    cout << "catch a signal, the signo is : " << signo << endl;
    count(20);
}

int main()
{
    struct sigaction act, oldact;
    act.sa_flags = 0;
    act.sa_handler = myhandler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGQUIT); // 将三号信号添加到屏蔽集中
    sigaction(SIGINT, &act, &oldact);

    while(true);
    return 0;
}
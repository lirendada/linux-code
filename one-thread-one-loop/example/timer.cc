#include <iostream>
#include <sys/timerfd.h>
#include <unistd.h>
using namespace std;

int main()
{
    // 1. 创建定时器
    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(timerfd == -1)
    {
        cerr << "timerfd_create error" << endl;
        return -1;
    }

    // 2. 设置定时器
    struct itimerspec newtimer;
    newtimer.it_value.tv_sec = 10;    // 设置第一次超时的时间
    newtimer.it_value.tv_nsec = 0;
    newtimer.it_interval.tv_sec = 2; // 设置第一次超时后每次的超时间隔时间
    newtimer.it_interval.tv_nsec = 0;
    timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &newtimer, nullptr);

    // 3. 打印测试定时情况
    time_t presec = time(nullptr);
    while(true)
    {
        uint64_t timer; // 比如是8个字节的变量
        int ret = read(timerfd, &timer, 8);
        if(ret < 0)
        {
            cerr << "read error" << endl;
            return -1;
        }
        cout << "超时了！当前已经累积" << time(nullptr) - presec << "秒" << endl;
    }
    close(timerfd);
    return 0;
}
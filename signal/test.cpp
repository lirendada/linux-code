#include <iostream>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
int main()
{
    while(true)
    {
        cout << "我是一个正在运行的进程，pid: " << getpid() << endl;
        sleep(1);
    }
    return 0;
}
// #include <iostream>
// #include <cstdlib>
// #include <string>
// #include <cstring>
// #include <unistd.h>
// #include <signal.h>
// #include <sys/types.h>
// using namespace std;

// // 自定义处理信号的函数
// void signal_handler(int signo)
// {
//     cout << "进程捕捉到一个信号，信号编码为：" << signo << endl;
// }

// int main()
// {
//     // signal(2, SIG_DFL); // 注册处理函数，告诉2号信号默认处理
//     // signal(2, SIG_IGN); // 注册处理函数，告诉2号信号也就是ctrl+c设为忽略动作
//     signal(2, signal_handler); // 注册处理函数，告诉2号信号通过自定义函数处理
//     while(true)
//     {
//         sleep(1);
//         cout << "i am a process, i am running, pid: " << getpid() << endl;
//     }
//     return 0;
// }

// static void Usage(const string& proc)
// {
//     cout << "\nUsage: "  << proc << " pid signo\n" << endl;
// }

// int main(int argc, char* argv[])
// {
//     if(argc != 3)
//     {
//         Usage(argv[0]);
//         exit(1);
//     }
//     // 通过kill系统调用函数来发送信号
//     pid_t pid = atoi(argv[1]);
//     int sig = atoi(argv[2]);
//     int n = kill(pid, sig);
//     if(n != 0)
//     {
//         perror("kill");
//     }
//     return 0;
// }

// int main(int argc, char* argv[])
// {
//     int cnt = 1;
//     while(true)
//     {
//         cout << "cnt = " << cnt++ << endl;
//         sleep(1);

//         // 调用raise给当前进程发送信号
//         if(cnt >= 6)
//             raise(8); // 相当于是kill(pid, 8);
//     }
//     return 0;
// }

// int main(int argc, char* argv[])
// {
//     int cnt = 1;
//     while(true)
//     {
//         cout << "cnt = " << cnt++ << endl;
//         sleep(1);

//         // 调用abort给当前进程发送六号信号
//         if(cnt >= 6)
//             abort(); // 相当于是kill(getpid(), 6)或者arise(6);
//     }
//     return 0;
// }

// void catchSig(int signo)
// {
//     cout << "catch a sign, the signo is " << signo << endl;
// }

// int main()
// {
//     // 通过自定义函数来捕捉这个信号
//     signal(SIGFPE, catchSig);
//     int a = 10;
//     a /= 0; // 除以0的情况
//     while(true)
//     {
//         cout << "我是一个运行中的进程......" << endl;
//         sleep(1);
        
//     }
//     return 0;
// }

// int cnt = 0;
// void catchSig(int signo)
// {
//     cout << "catch a sign, the cnt is " << cnt << endl;
//     alarm(1); // 因为主函数中闹钟只响应一次，所以这里再重新设置一遍闹钟
//     cnt = 0; // 并且将计数重新设为0
// }

// int main()
// {
//     // 注册自定义捕捉信号方法
//     signal(SIGALRM, catchSig);

//     // 设置闹钟
//     alarm(1);
    
//     while(true)
//     {
//         cnt++;
//     }
//     return 0;
// }

// int main()
// {
//     signal(SIGSEGV, SIG_DFL);

//     // 核心转储
//     int arr[10];
//     arr[100000] = 10; // 会发生段错误
    
//     return 0;
// }

#include <iostream>
#include <signal.h>
#include <vector>
#include <unistd.h>
using namespace std;

static vector<int> catchSig{2, 3}; // 要阻塞的信号编号数组

void myhandler(int signo)
{
    cout << "捕捉信号，编号为：" << signo << endl;
}

static void show_pending(sigset_t& pending)
{
    // 注意信号编号是从1开始的
    for(int i = 31; i >= 1; --i)
    {
        // 判断该信号是否为未决信号
        if(sigismember(&pending, i))
            cout << "1";
        else 
            cout << "0";
    }
    cout << endl;
}

int main()
{
    // 自定义捕捉信号
    for(const auto e : catchSig)
        signal(e, myhandler);

    // 1.先屏蔽指定的信号
    // 1.1 定义阻塞信号集
    sigset_t block, oldblock;
    // 1.2 初始化阻塞信号集
    sigemptyset(&block);
    sigemptyset(&oldblock);
    // 1.3 添加要屏蔽的信号
    for(const auto e : catchSig)
        sigaddset(&block, e);
    // 1.4 更改当前进程的阻塞信号集（这一步才是真正的设置到内核，上面都只是用户级别操作）
    sigprocmask(SIG_SETMASK, &block, &oldblock);

    // 2.打印pending信号集观察
    // 2.1 初始化
    sigset_t pending;
    sigemptyset(&pending);
    int cnt = 10;
    while(true)
    {
        // 2.2 获取进程的pending到我们的pending变量
        sigpending(&pending);
        // 2.3 打印
        show_pending(pending);
        sleep(1);

        // 十秒到后我们将其阻塞去掉
        if(cnt-- == 0)
        {
            // 我们可以通过sigdelset，也可以像下面这样子来讲阻塞去掉
            sigprocmask(SIG_SETMASK, &oldblock, &block);
            cout << "阻塞去掉，信号被捕捉" << endl;
        }
    }
    return 0;
}
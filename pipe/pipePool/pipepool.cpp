#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cassert>
#include <ctime>
using namespace std;

#define MakeSeed() srand((unsigned long)time(nullptr) ^ getpid() ^ 0x171237 ^ rand() % 1234)

///////////////////////////////////////中间部分为任务功能部分/////////////////////

typedef void(* func_t)(); // 函数指针

void IOTask()
{
    cout << getpid() << " : IO Task running\n" << endl;
    sleep(1);
}

void DownloadTask()
{
    cout << getpid() << " : Download Task running\n" << endl;
    sleep(1);
}

void flushTask()
{
    cout << getpid() << " : flush Task running\n" << endl;
    sleep(1);
}

void LoadFunction(vector<func_t>* funcMap)
{
    assert(funcMap);
    funcMap->push_back(IOTask);
    funcMap->push_back(DownloadTask);
    funcMap->push_back(flushTask);
}

///////////////////////////////////////以下为管道池的管理////////////////////////

const int PROCESS_NUM = 5; // 子进程池的个数

// 描述每对子进程和管道文件的结构体
class childProcess
{
public:
    childProcess(pid_t pid, int writeFd)
        :_pid(pid), _writeFd(writeFd)
    {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "process-%d[pid(%d)|writeFd(%d)]", num++, _pid, _writeFd);
        _name = buffer;
    }
public:
    string _name; // 以统一的规则命名的名称
    pid_t _pid; // 子进程pid
    int _writeFd; // 管道文件的写入端
    static int num; // 当前子进程为第几个进程编号
};

int childProcess::num = 0;

void SendTask(const childProcess& cp, int index_func)
{
    cout << "Send task num: " << index_func << " to -> " << cp._name << endl;

    // 发送任务就是向管道里写入数据
    int n = write(cp._writeFd, &index_func, sizeof(index_func));
    assert(n == sizeof(int)); // 断言一下，发送的字节大小必须为int类型大小
    (void)n;
}

int ReceiveTask(int readFd)
{
    int code = 0;
    ssize_t n = read(readFd, &code, sizeof(code));
    if(n == 4) 
        return code; 
    else if(n <= 0)
        return -1;
    else 
        return 0;
}

void CreateProcessPool(vector<childProcess>* pipePool, vector<func_t>& funcMap)
{
    vector<int> deleteFd;
    for(int i = 0; i < PROCESS_NUM; ++i)
    {
        // 父进程生成管道文件
        int pipefd[2];
        int n = pipe(pipefd);
        assert(n != -1); // 断言一下管道是否生成
        (void)n; // 防止release报错

        pid_t id = fork();
        assert(id >= 0); // 断言一下是否fork成功

        // 子进程的执行部分
        if(id == 0)
        {
            // 每次删掉子进程拷贝父进程的前n个写入端指向
            for(int i = 0; i < deleteFd.size(); ++i) 
                close(deleteFd[i]);

            close(pipefd[1]); // 子进程进行读取，所以关闭写入端
            while(true) // 循环执行，一直等待读取父进程的信号
            {
                // 1、获取命令码，如果没有收到则一直阻塞等待
                int commandCode = ReceiveTask(pipefd[0]);
                // 2、完成任务，只有命令名符合要求才能执行任务
                if(commandCode >= 0 && commandCode < funcMap.size())
                    funcMap[commandCode]();
                else if(commandCode == -1)
                    break;
            }
            exit(0);
        }

        // 父进程的执行部分
        close(pipefd[0]); // 父进程进行写入，所以关闭读取端
        childProcess cp(id, pipefd[1]); // 创建一个childProcess对象
        pipePool->push_back(move(cp)); // 将该对象调用move移动构造到管道池
        deleteFd.push_back(pipefd[1]); // 记录每个子进程的前n个写入端fd
    }
}

void loadBlanceContrl(vector<childProcess>& cp, vector<func_t>& fmp, int taskCnt)
{
    int pipe_size = cp.size(); // 管道池个数
    int func_size = fmp.size(); // 任务个数
    bool forever = (taskCnt == 0 ? true : false); // 判断是否为永远

    while(true)
    {
        // 1、选择一个子进程
        int index_process = rand() % pipe_size;
        // 2、选择一个任务
        int index_func = rand() % func_size;
        // 3、任务发送给选择的进程
        SendTask(cp[index_process], index_func);
        sleep(1);

        if(!forever)
        {
            taskCnt--;
            if(taskCnt == 0) break;   
        }
    }

    // 关闭写入端
    for(int i = 0; i < PROCESS_NUM; ++i)
        close(cp[i]._writeFd); // 类似堆栈的方式的原理关闭的写入端
}

void waitProcess(vector<childProcess>& cp)
{
    for(int i = 0; i < PROCESS_NUM; ++i)
    {
        // 这里不做退出码等处理
        waitpid(cp[i]._pid, nullptr, 0);
        cout << "wait child_process success -> " << cp[i]._pid << endl;
    }
}

int main()
{
    MakeSeed(); // 随机种子

    // 1、创建管道进程池和任务列表
    vector<func_t> funcMap;
    LoadFunction(&funcMap);

    vector<childProcess> pipePool;
    CreateProcessPool(&pipePool, funcMap);

    // 2、父进程控制子进程完成任务，负载均衡的向子进程发送命令码，若父进程退出则关闭子进程
    int taskCnt = 3; // 0: 永远进行，其它则表示计数
    loadBlanceContrl(pipePool, funcMap, taskCnt);
    
    // 3、回收子进程信息
    waitProcess(pipePool);

    return 0;
}
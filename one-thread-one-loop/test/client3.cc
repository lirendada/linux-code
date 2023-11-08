#include "../source/server.hpp"

/*  业务处理超时，查看服务器的处理情况：
    因为当服务器达到了一个性能瓶颈，如果在一次业务处理中花费了太长的时间，（有可能超过了服务器设置的非活跃超时时间）
    则有可能导致其他的连接也被连累超时，其他的连接有可能会被拖累超时释放！
      假设现在 12345 描述符就绪了且非活跃超时时间为30s，在处理 1 的时候花费了30s处理后，导致 2345 描述符因为长时间没有刷新活跃度，此时有两种情况：
        1. 如果接下来的 2345 描述符都是通信连接描述符，如果都就绪了，则没有太大的影响，因为接下来就会进行处理并刷新活跃度
        2. 如果接下来的 2 号描述符是定时器事件描述符，此时定时器触发超时，执行定时任务，就会将 345 描述符给释放掉
           这时候一旦 345 描述符对应的连接被释放，接下来在处理 345 事件的时候就会导致程序崩溃（内存访问错误）
           因此这时候，在本次事件处理中，并不能直接对连接进行释放，而应该将释放操作压入到任务池中，等到事件处理完了执行任务池中的任务的时候，再去释放
*/
int main()
{
    // 创建多进程客户端套接字
    signal(SIGCHLD, SIG_IGN);
    for(int i = 0; i < 10; ++i)
    {
        pid_t id = fork();
        if(id < 0)
        {
            ELOG("fork error");
            return -1;
        }
        else if(id == 0)
        {
            // 子进程进行数据发送
            Socket client_sock;
            client_sock.create_client(8080, "81.71.97.127");
            std::string str = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\n\r\n";
            while(true)
            { 
                assert(client_sock.Send(str.c_str(), str.size()) != -1);
                char buf[1024] = { 0 };
                client_sock.Recv(buf, sizeof(buf) - 1);
                DLOG("%s", buf);
                sleep(1);
            }
            exit(0);
        }
    }
    while(true); // 父进程进行死循环
    return 0;
}
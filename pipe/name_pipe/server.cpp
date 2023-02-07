#include "commom.hpp"
int main()
{
    // 创建管道
    bool flag = createFifo(NAMED_PIPE);
    assert(flag);
    (void)flag;

    // 打开文件后，接收信息，最后关闭文件
    std::cout << "server begin" << std::endl;
    int rfd = open(NAMED_PIPE, O_RDONLY);
    std::cout << "server end" << std::endl;
    if(rfd < 0)
        exit(1);

    char buffer[1024];
    while(true)
    {
        ssize_t n = read(rfd, buffer, sizeof(buffer) - 1);
        if(n > 0)
        {
            buffer[n] = '\0';
            std::cout << "client->server# " << buffer << std::endl;
        }
        else if(n == 0)
        {
            std::cout << "client quit, me too!" << std::endl;
            break;
        }
        else
        {
            std::cout << "errno: " << errno << " err string: " << strerror(errno) << std::endl;
            break;
        }
    }

    close(rfd);
    // 删除管道
    removeFifo(NAMED_PIPE);
    return 0;
}
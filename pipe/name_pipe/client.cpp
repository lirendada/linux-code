#include "commom.hpp"
int main()
{
    // 打开文件后发送信息，最后关闭文件
    std::cout << "client begin" << std::endl;
    int wfd = open(NAMED_PIPE, O_WRONLY | O_TRUNC, 0666);
    std::cout << "client end" << std::endl;
    if(wfd < 0) 
        exit(1);

    char buffer[1024];
    while(true)
    {
        std::cout << "Please Say# ";
        fgets(buffer, sizeof(buffer), stdin); // 从键盘写到buffer
        buffer[strlen(buffer) - 1] = '\0'; // 将最后的回车去掉，不必担心会得到buffer[-1]，因为我们始终要按一次回车

        // 将buffer写入文件
        ssize_t n = write(wfd, buffer, strlen(buffer)); 
        assert(n == strlen(buffer));
        (void)n;
    }

    close(wfd);
    return 0;
}
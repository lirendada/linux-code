#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main()
{
    // C语言文件接口
    printf("hello lirendada:print\n");
    fprintf(stdout, "hello lirendada:fprint\n");
    const char* fputsStr = "hello lirendada:fputs\n";
    fputs(fputsStr, stdout);

    // 系统接口
    const char* str = "hello write\n";
    write(stdout->_fileno, str, strlen(str));

    // 只调用fork，什么都不做
    fork();
    return 0;
}
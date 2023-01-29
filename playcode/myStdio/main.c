#include "myStdio.h"
#include <stdio.h> // 为了打印测试
 int main ()
{
    _FILE* fp = _fopen("./log.txt","w");//传入路径名和刷新模式
    if(fp == NULL)
        return 1;
 
    int cnt = 10;
    const char *msg = "lirendada!\n";
    while(1)
    {
        _fwrite(fp, msg, strlen(msg));
        //fflush_(fp);
        sleep(1);
        printf("count: %d\n", cnt);
        cnt--;
        if(cnt == 0) break;
    }
 
    _fclose(fp);
    return 0;
}
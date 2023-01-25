#include <stdio.h>
#include <unistd.h>
int main()
{
    extern char** environ; // 最好要声明一下
    for(int i = 0; environ[i]; ++i)
    {
        printf("%d:%s\n", i, environ[i]);
    }
    return 0;
}

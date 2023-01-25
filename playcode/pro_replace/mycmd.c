#include <stdio.h>
#include <stdlib.h>

int main()
{
    for(int i = 0; i < 5; ++i)
        printf("cmd：%d\n", i);
    printf("MYENV：%s\n", getenv("MYENV"));
    
    printf("PATH：%s\n", getenv("PATH"));
    printf("PWD：%s\n", getenv("PWD"));
    return 0;
}

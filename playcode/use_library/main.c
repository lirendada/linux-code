#include "add.h"
#include "sub.h"
int main()
{
    printf("add：%d + %d = %d\n", 10, 20, add(10, 20));
    printf("sub：%d - %d = %d\n", 10, 20, sub(10, 20));
    return 0;
}
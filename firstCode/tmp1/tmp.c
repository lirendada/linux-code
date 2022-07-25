#include "tmp.h"

int SumToNum(int start, int end)
{
    int sum = 0;
    for(int i = start; i <= end; ++i)
    {
      sum += i;
    }
    return sum;
}

int main()
{
    int start = 1;
    int end = 100;
    int sum = SumToNum(start, end);
    printf("sum = %d\n", sum);
}

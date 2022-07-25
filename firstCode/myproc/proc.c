#include "proc.h"

void Count()
{
    int i = 15;
    while(i)
    {
        printf("%-2d\r",i);
        fflush(stdout);
        sleep(1);
        i--;
    }
}

void ProcBar()
{
    int i = 0;
    char proc[102] = {0};
    const char* tmp = "|/-\\";
    while(i <= 100)
    {
        printf("[%-100s][%d%%][%c]\r", proc, i, tmp[i%4]);    
        fflush(stdout);
        proc[i] = '#';
        usleep(55555);
        ++i;
    }
    printf("\n");
}

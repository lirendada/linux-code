#include "comm.hpp"

int main()
{
    key_t k = getKey();
    printf("key: 0x%x\n", k);

    int shmid = getShm(k);
    printf("shmid: %d\n", shmid);
    return 0;
}
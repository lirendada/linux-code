#include "comm.hpp"

int main()
{
    key_t k = getKey();
    printf("0x%x\n", k);

    int shmid = createShm(k);
    printf("shmid: %d\n", shmid);

    // 一般谁创建shm，谁删除shm！
    delShm(shmid);
    return 0;
}
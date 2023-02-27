#include "comm.hpp"

int main()
{
    key_t k = getKey();
    printf("0x%x\n", k);

    int shmid = createShm(k);
    printf("shmid: %d\n", shmid);

    // 关联该共享内存
    char* start = (char*)attachShm(shmid);
    printf("attach success, address start: %p\n", start);
    sleep(5);

    // 使用
    while(true)
    {
        sleep(1);
        // 不需要读取到数组中，因为可以直接通过共享内存地址start打印出来
        printf("client say: %s\n", start);

        // 调用shmctl获取共享内存的属性
        struct shmid_ds ds;
        shmctl(shmid, IPC_STAT, &ds);
        printf("获取属性: size: %d, pid: %d, myself: %d, key: 0x%x\n",\
                ds.shm_segsz, ds.shm_cpid, getpid(), ds.shm_perm.__key);
    }

    // 去关联
    detachShm(start);
    sleep(5);

    // 一般谁创建shm，谁删除shm！
    delShm(shmid);
    return 0;
}
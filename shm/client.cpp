#include "comm.hpp"

int main()
{
    key_t k = getKey();
    printf("key: 0x%x\n", k);

    int shmid = getShm(k);
    printf("shmid: %d\n", shmid);

    // 关联该共享内存
    char* start = (char*)attachShm(shmid);
    printf("attach success, address start: %p\n", start);
    sleep(3);

    // 使用
    pid_t id = getpid();
    int cnt = 1;
    const char* str = "i am client! i am talking with you!";
    while(true)
    {
        sleep(1);
        // 不需要先加载到字符数组中，直接可以通过共享内存地址写入即可
        snprintf(start, MAX_SIZE, "%s : [pid: %d][cnt: %d]", str, id, cnt++);
    }

    // 去关联
    detachShm(start);
    sleep(3);
    return 0;
}
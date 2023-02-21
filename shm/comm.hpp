#ifndef _COMM_HPP_
#define _COMM_HPP_

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
using namespace std;

// 设置两个常量pathname和proj_id，用来获取key值
const char* PATHNAME = ".";
const int PROJ_ID = 0x66;

const int MAX_SIZE = 4096; // 共享内存的大小

// 调用ftok函数获得key
key_t getKey()
{
    key_t k = ftok(PATHNAME, PROJ_ID);
    if(k == -1)
    {
        cerr << errno << " : " << strerror(errno) << endl;
        exit(1);
    }
    return k;
}

// 创建或者获取共享内存标识符的函数
int getShmHelper(key_t k, int flags)
{
    int shmid = shmget(k, MAX_SIZE, flags);
    if(shmid == -1)
    {
        cerr << errno << " : " << strerror(errno) << endl;
        exit(2);
    }
    return shmid;
}

// 为client提供获取共享内存标识符的函数
int getShm(key_t k)
{
    return getShmHelper(k, IPC_CREAT);
}

// 为server提供创建共享内存标识符的函数
int createShm(key_t k)
{
    return getShmHelper(k, IPC_CREAT | IPC_EXCL);
}

// 删除共享内存
void delShm(int shmid)
{
    if(shmctl(shmid, IPC_RMID, nullptr) == -1)
    {
        cerr << errno << " : " << strerror(errno) << endl;
        exit(3);
    }
}

#endif
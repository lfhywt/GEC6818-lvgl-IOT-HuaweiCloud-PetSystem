#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

// 定义共享的结构体
typedef struct share {
    int light_flag;
    int kong_flag;
    int camera_flag;
    int feed_flag;
} SharedData;

// 共享内存的键值（两个程序要一样）
#define SHM_KEY 1234

#endif

#include "led.h"
#include "ui_onoff_btn.h"
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// 全局或外部变量
extern bool light_flag;
extern bool kong_flag;
extern bool camera_flag;
extern bool feed_flag;

// 蜂鸣器响一次的线程函数
void *press_buz_thread(void *arg)
{
    int fd_buz = open("/dev/beep", O_RDWR);
    if (fd_buz < 0)
    {
        perror("open beep");
        pthread_exit(NULL);
    }

    ioctl(fd_buz, ON, 1);   // 打开蜂鸣器
    usleep(80000);         // 响 80ms
    ioctl(fd_buz, OFF, 1);   // 关闭蜂鸣器

    close(fd_buz);
    pthread_exit(NULL);
}

// LED 主控制函数
// led.c 修正后
int LED(volatile sig_atomic_t *exit_flag)  // 接收退出标志指针
{
    int fd = open("/dev/Led", O_RDWR);
    if (fd < 0) {
        perror("open Led");
        return -1;
    }

    // 保存上一次状态（用于检测变化）
    static int prev_light = -1, prev_kong = -1, prev_camera = -1, prev_feed = -1;

    // 只执行一次状态更新
    light_flag ? ioctl(fd, LED1, LED_ON) : ioctl(fd, LED1, LED_OFF);
    kong_flag ? ioctl(fd, LED2, LED_ON) : ioctl(fd, LED2, LED_OFF);
    camera_flag ? ioctl(fd, LED3, LED_ON) : ioctl(fd, LED3, LED_OFF);
    feed_flag ? ioctl(fd, LED4, LED_ON) : ioctl(fd, LED4, LED_OFF);

    // 检测状态变化并触发蜂鸣器
    if (light_flag != prev_light || kong_flag != prev_kong ||
        camera_flag != prev_camera || feed_flag != prev_feed) {
        pthread_t tid_press_buz;
        pthread_create(&tid_press_buz, NULL, press_buz_thread, NULL);
        pthread_detach(tid_press_buz);

        // 更新上次状态
        prev_light = light_flag;
        prev_kong = kong_flag;
        prev_camera = camera_flag;
        prev_feed = feed_flag;
    }

    close(fd);  // 每次执行完立即关闭，避免文件描述符泄漏
    return 0;
}
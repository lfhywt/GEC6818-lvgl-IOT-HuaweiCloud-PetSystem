#ifndef __HOME_H_
#define __HOME_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>     // man 2 socket    /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>    //man 3 inet_addr
#include <arpa/inet.h>
#include <dirent.h>


#define SERVER_IP   "192.168.171.60"  //MY_IP OWN_IP
#define SERVER_PORT   10000

// #define SAVE_DIR "S:/IOT/photo/"
extern volatile bool recv_thread_running;
extern int g_sock;
extern pthread_mutex_t g_sock_lock;

int thread_created;

// 启动与主屏

void load_animal_smart_home_screen(void); // 加载智能家居主界面
void animal_screen_exit(void);            // 退出智能家居主界面
void photo_event_auto(lv_event_t * e);    // 自动播放事件
void photo_event_manual(lv_event_t * e);  // 手动播放事件
void switch_to_zhuce(lv_timer_t * timer); // 延迟跳转注册页回调
void lv_zhuce(void);
void switch_img_cb(lv_timer_t * timer); // 图片切换回调
void game_event_manual(lv_event_t * e);
void * recv_client_info(void * arg);
void refresh_image_screen(void);
void * connect_server_thread(void * arg);

#endif
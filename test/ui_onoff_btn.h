#ifndef __ONOFF_H_
#define __ONOFF_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include <stdio.h>
pthread_mutex_t flags_mutex;
pthread_cond_t need_send_cond;
extern int ctrl_threads_created;           // 线程是否已创建
extern volatile sig_atomic_t exit_threads; // 线程退出标志
void on_off_event_li(lv_event_t * e);      // 开关事件
void lv_li_on_off(void);                   // 开关页面
void on_off_page_li(void);                 // 开关页面内容

// 其他控件事件
void light_event_li(lv_event_t * e);  // 灯光事件
void kong_event_li(lv_event_t * e);   // 空调事件
void camera_event_li(lv_event_t * e); // 摄像头事件
void feed_event_li(lv_event_t * e);   // 喂食事件
void * report_status_thread(void * arg);
void update_ui_state(void);
void stop_ctrl_thread(void);
void stop_report_thread(void);
void on_off_page_exit(void);

extern bool light_flag;
extern bool kong_flag;
extern bool camera_flag;
extern bool feed_flag;


#endif
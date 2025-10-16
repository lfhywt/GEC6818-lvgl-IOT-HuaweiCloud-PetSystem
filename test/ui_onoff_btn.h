#ifndef __ONOFF_H_
#define __ONOFF_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

void on_off_event_li(lv_event_t * e);       // 开关事件
void lv_li_on_off(void);                           // 开关页面
void on_off_page_li(void);                         // 开关页面内容

// 其他控件事件
 void light_event_li(lv_event_t * e);        // 灯光事件
 void kong_event_li(lv_event_t * e);         // 空调事件
 void camera_event_li(lv_event_t * e);       // 摄像头事件
 void feed_event_li(lv_event_t * e);         // 喂食事件

#endif
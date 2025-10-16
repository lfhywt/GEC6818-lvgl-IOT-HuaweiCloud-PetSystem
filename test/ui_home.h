#ifndef __HOME_H_
#define __HOME_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

// 启动与主屏

void load_animal_smart_home_screen(void); // 加载智能家居主界面
void animal_screen_exit(void);            // 退出智能家居主界面
void photo_event_auto(lv_event_t * e);    // 自动播放事件
void photo_event_manual(lv_event_t * e);  // 手动播放事件
void switch_to_zhuce(lv_timer_t * timer); // 延迟跳转注册页回调
void lv_zhuce(void);
void switch_img_cb(lv_timer_t * timer); // 图片切换回调
void game_event_manual(lv_event_t * e);

#endif
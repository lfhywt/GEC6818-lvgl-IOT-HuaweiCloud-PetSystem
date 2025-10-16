#ifndef __UPTIME_H
#define __UPTIME_H

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

//extern lv_obj_t * img3; /**< 临时图片对象 */


// ------------------------ 函数声明 ------------------------
static void progress_timer_cb(lv_timer_t * timer); // 进度条定时器回调
void lv_li_kaiji(void);                     // 启动页（加载动画）
void lv_zhuce(void);
// 工具与通用
lv_obj_t * show_image(const char * img_path, lv_coord_t x, lv_coord_t y); // 显示图片







#endif
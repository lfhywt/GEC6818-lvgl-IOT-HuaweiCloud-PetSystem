#ifndef __TEMP_H_
#define __TEMP_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

// 温控与开关页面
lv_obj_t * temp_control_create(lv_obj_t * parent); // 创建温控控件
void lv_li_temp(void);                             // 温控页面
void temp_event_li(lv_event_t * e);         // 温控事件

#endif
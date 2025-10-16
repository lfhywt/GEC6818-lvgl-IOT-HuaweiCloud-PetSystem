#ifndef __BTN_H_
#define __BTN_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>


extern lv_obj_t * label;                    /**< 百分比标签 */
static void login_event_cb1(lv_event_t * e);
void button_back(void);                                                   // 返回按钮
#endif
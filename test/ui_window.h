#ifndef __WINDOW_H_
#define __WINDOW_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

void window_page_li(void);                // 窗户页面
 void window_event_li(lv_event_t * e); // 窗户按钮事件

#endif
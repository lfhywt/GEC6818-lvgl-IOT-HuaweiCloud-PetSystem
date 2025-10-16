#ifndef __INFO_H_
#define __INFO_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

// 信息与窗户页面
void information_page_li(void);           // 信息页
void information_event_li(lv_event_t * e); // 信息页按钮事件
char * read_text_from_file(const char * path);
#endif
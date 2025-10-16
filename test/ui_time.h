#ifndef __TIME_H_
#define __TIME_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>

 static lv_obj_t * time_label = NULL;        /**< 时间标签 */
static lv_timer_t * time_timer = NULL;      /**< 时间更新定时器 */
 struct tm custom_time;               /**< 自定义时间缓存 */
void create_time_widget(void);                                            // 创建时间控件
void time_update_cb(lv_timer_t * timer);

#endif
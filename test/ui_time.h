#ifndef UI_TIME_H
#define UI_TIME_H

#include "lvgl/lvgl.h"
#include <time.h>  // 用于struct tm

// 外部声明（供其他文件访问）
extern lv_obj_t *time_label;        // 时间显示标签
extern lv_timer_t *time_timer;      // 时间更新定时器
extern struct tm custom_time;       // 自定义时间缓存

// 函数声明
void create_time_widget(void);      // 创建时间控件
void time_update_cb(lv_timer_t *timer);  // 时间更新回调

#endif // UI_TIME_H
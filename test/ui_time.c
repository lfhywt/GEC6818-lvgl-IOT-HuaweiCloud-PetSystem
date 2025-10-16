#include "ui_time.h"
#include "lv_font_source_han_sans_bold.h"

// -------------------- 时间控件 --------------------

void time_update_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    if(time_label == NULL) return;

    // 每次 +1 秒
    custom_time.tm_sec++;
    if(custom_time.tm_sec >= 60) {
        custom_time.tm_sec = 0;
        custom_time.tm_min++;
    }
    if(custom_time.tm_min >= 60) {
        custom_time.tm_min = 0;
        custom_time.tm_hour++;
    }
    if(custom_time.tm_hour >= 24) {
        custom_time.tm_hour = 0;
    }

    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", &custom_time);
    lv_label_set_text(time_label, buf);
}

void create_time_widget(void)
{
    if(time_label != NULL) return;

    lv_obj_t * top_layer = lv_layer_top();
    time_label           = lv_label_create(top_layer);
    lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, -10, 6);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);

    // 设置初始时间 08:30:00
    memset(&custom_time, 0, sizeof(custom_time));
    custom_time.tm_hour = 8;
    custom_time.tm_min  = 30;
    custom_time.tm_sec  = 0;

    // 显示初始值
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M:%S", &custom_time);
    lv_label_set_text(time_label, buf);

    // 启动定时器
    time_timer = lv_timer_create(time_update_cb, 1000, NULL);
}

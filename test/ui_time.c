#include "ui_time.h"
#include "lv_font_source_han_sans_bold.h" // 字体支持
#include "ntp_client.h"                   // 新增：引入NTP客户端接口
#include <stdio.h>
#include <string.h>

// 全局变量定义（仅在当前文件初始化）
lv_obj_t * time_label   = NULL;
lv_timer_t * time_timer = NULL;
struct tm custom_time;

/**
 * 时间更新回调函数：每秒更新一次时间显示
 */
void time_update_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    if(time_label == NULL) return; // 防止空指针

    // 时间递增逻辑（秒->分->时）
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
        custom_time.tm_hour = 0; // 小时归零（可扩展日期逻辑）
    }

    // 格式化时间字符串（HH:MM:SS）
    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &custom_time);
    lv_label_set_text(time_label, time_buf);
}

/**
 * 创建时间控件：从NTP同步时间（失败则用默认值），在顶部右侧显示并启动更新
 */
void create_time_widget(void)
{
    if(time_label != NULL) return; // 避免重复创建

    // 1. 创建时间标签（放在顶层图层，不被其他控件遮挡）
    lv_obj_t * top_layer = lv_layer_top(); // LVGL顶层图层
    time_label           = lv_label_create(top_layer);

    // 2. 设置标签样式（位置、颜色、字体）
    lv_obj_align(time_label, LV_ALIGN_TOP_RIGHT, -10, 6);                          // 右上角偏移
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // 白色文字
    lv_obj_set_style_text_font(time_label, &chinese_ziku, LV_PART_MAIN);           // 应用中文字体

    // 3. 初始化时间（优先从NTP服务器同步，失败则使用默认值08:30:00）
    memset(&custom_time, 0, sizeof(custom_time));
    time_t ntp_time = get_ntp_time("192.168.171.60"); // 调用NTP接口获取时间（阿里云服务器）

    if(ntp_time != (time_t)-1) {
        // NTP获取成功：转换为本地时间并初始化
        struct tm * ntp_tm = localtime(&ntp_time);
        if(ntp_tm != NULL) {
            custom_time = *ntp_tm; // 复制NTP时间到自定义时间结构
                                   // 关键：若服务器返回UTC时间，手动加8小时转为北京时间（UTC+8）
            custom_time.tm_hour += 8;
        } else {
            // localtime转换失败，使用默认时间
            fprintf(stderr, "Failed to convert NTP time, use default\n");
            custom_time.tm_hour = 8;
            custom_time.tm_min  = 30;
            custom_time.tm_sec  = 0;
        }
    } else {
        // NTP获取失败，使用默认时间
        fprintf(stderr, "Failed to get NTP time, use default\n");
        custom_time.tm_hour = 8;
        custom_time.tm_min  = 30;
        custom_time.tm_sec  = 0;
    }

    // 4. 显示初始时间
    char init_time[16];
    strftime(init_time, sizeof(init_time), "%H:%M:%S", &custom_time);
    lv_label_set_text(time_label, init_time);

    // 5. 启动定时器（每秒触发一次更新）
    time_timer = lv_timer_create(time_update_cb, 1000, NULL);
}
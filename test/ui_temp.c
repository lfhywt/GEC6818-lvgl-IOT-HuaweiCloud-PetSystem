#include "ui_temp.h"
#include "lv_font_source_han_sans_bold.h"

// -------------------- 温度控件 --------------------
#define TEMP_MIN 0
#define TEMP_MAX 50

typedef struct
{
    lv_obj_t * cont;
    lv_obj_t * arc;
    lv_obj_t * slider;
    lv_obj_t * lbl_value;
    lv_obj_t * lbl_unit; /* 用作 "°C" */
} temp_control_t;

/* 颜色映射（不变） */
static lv_color_t temp_to_color(int t)
{
    if(t <= 20) {
        int r = 30 + (70) * (t - TEMP_MIN) / (20 - TEMP_MIN + 1);
        int g = 120 + (60) * (t - TEMP_MIN) / (20 - TEMP_MIN + 1);
        int b = 240 + (15) * (t - TEMP_MIN) / (20 - TEMP_MIN + 1);
        return lv_color_make((uint8_t)r, (uint8_t)g, (uint8_t)b);
    } else if(t <= 35) {
        int r = 100 + (155) * (t - 20) / (35 - 20 + 1);
        int g = 180 + (20) * (t - 20) / (35 - 20 + 1);
        int b = 255 - (205) * (t - 20) / (35 - 20 + 1);
        return lv_color_make((uint8_t)r, (uint8_t)g, (uint8_t)b);
    } else {
        int r = 255 - (35) * (t - 35) / (TEMP_MAX - 35 + 1);
        int g = 200 - (160) * (t - 35) / (TEMP_MAX - 35 + 1);
        int b = 50 - (10) * (t - 35) / (TEMP_MAX - 35 + 1);
        return lv_color_make((uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
}

/* 统一设置值（避免回调互相触发） */
static void set_value_internal(temp_control_t * tc, int val)
{
    if(!tc) return;
    if(val < TEMP_MIN) val = TEMP_MIN;
    if(val > TEMP_MAX) val = TEMP_MAX;

    /* 以编程方式设置控件值并更新显示（LV_ANIM_OFF） */
    lv_slider_set_value(tc->slider, val, LV_ANIM_OFF);
    lv_arc_set_value(tc->arc, val);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", val);
    lv_label_set_text(tc->lbl_value, buf);

    /* 背景颜色 */
    lv_obj_set_style_bg_color(tc->cont, temp_to_color(val), 0);
}

/* slider 回调：用户拖动滑块时调用 */
static void slider_cb(lv_event_t * e)
{
    lv_obj_t * sld      = lv_event_get_target(e);
    temp_control_t * tc = (temp_control_t *)lv_event_get_user_data(e);
    int v               = lv_slider_get_value(sld);
    set_value_internal(tc, v);
}

/* arc 回调：用户拖动圆弧时调用 */
static void arc_cb(lv_event_t * e)
{
    lv_obj_t * a        = lv_event_get_target(e);
    temp_control_t * tc = (temp_control_t *)lv_event_get_user_data(e);
    int v               = lv_arc_get_value(a);
    set_value_internal(tc, v);
}

/* parent 可传 NULL（内部会用 lv_scr_act()） */
lv_obj_t * temp_control_create(lv_obj_t * parent)
{
    if(parent == NULL) parent = lv_scr_act();

    temp_control_t * tc = (temp_control_t *)malloc(sizeof(temp_control_t));
    if(!tc) return NULL;

    tc->cont = lv_obj_create(parent);
    lv_obj_set_size(tc->cont, 280, 280);
    lv_obj_set_style_radius(tc->cont, 12, 0);
    lv_obj_set_style_pad_all(tc->cont, 8, 0);
    lv_obj_set_style_bg_opa(tc->cont, LV_OPA_COVER, 0);
    lv_obj_clear_flag(tc->cont, LV_OBJ_FLAG_SCROLLABLE);

    /* arc（可交互） */
    tc->arc = lv_arc_create(tc->cont);
    lv_obj_set_size(tc->arc, 160, 160);
    lv_obj_center(tc->arc);
    lv_arc_set_range(tc->arc, TEMP_MIN, TEMP_MAX);
    lv_arc_set_value(tc->arc, 25);
    /* 设置样式（宽度与颜色） */
    lv_obj_set_style_arc_width(tc->arc, 12, 0);
    lv_obj_set_style_arc_color(tc->arc, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(tc->arc, lv_color_hex(0x202020), LV_PART_MAIN);
    /* 默认允许交互（拖动）——确保对象可点击 */
    lv_obj_clear_flag(tc->arc,
                      LV_OBJ_FLAG_CLICKABLE); /* 如果想禁用把这行改为 lv_obj_add_flag(..., LV_OBJ_FLAG_CLICKABLE) */
    /* 注意：有些平台/主题会影响是否显示旋钮，若看不到旋钮可通过样式绘制自定义指示器 */

    /* 创建显示数字（仅数字） */
    tc->lbl_value = lv_label_create(tc->cont);
    lv_obj_set_style_text_font(tc->lbl_value, LV_FONT_DEFAULT, 0);
    lv_label_set_text(tc->lbl_value, "25");
    lv_obj_align(tc->lbl_value, LV_ALIGN_CENTER, -8, -6);

    /* 单位 °C（保留） */
    tc->lbl_unit = lv_label_create(tc->cont);
    lv_obj_set_style_text_font(tc->lbl_unit, LV_FONT_DEFAULT, 0);
    lv_label_set_text(tc->lbl_unit, "°C");
    lv_obj_align_to(tc->lbl_unit, tc->lbl_value, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    /* slider（依然保留） */
    tc->slider = lv_slider_create(tc->cont);
    lv_obj_set_size(tc->slider, 160, 18);
    lv_obj_align(tc->slider, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_slider_set_range(tc->slider, TEMP_MIN, TEMP_MAX);
    lv_slider_set_value(tc->slider, 25, LV_ANIM_OFF);

    /* 添加回调并把 tc 当作 user_data 传入 */
    lv_obj_add_event_cb(tc->slider, slider_cb, LV_EVENT_VALUE_CHANGED, tc);
    lv_obj_add_event_cb(tc->arc, arc_cb, LV_EVENT_VALUE_CHANGED, tc);

    /* 初始显示 */
    set_value_internal(tc, 25);

    return tc->cont;
}


// ------------------------ 温控页事件 ------------------------
 void temp_event_li(lv_event_t * e)
{
    animal_screen_exit();
    ui_load_page(lv_li_temp);
}

// ------------------------ 温控页面 ------------------------
void lv_li_temp(void)
{
    // 背景图片
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img1, 0, 0);
    lv_img_set_src(img1, "S:/IOT/background5.png");

    lv_obj_t * ctrl = temp_control_create(lv_scr_act());
    lv_obj_center(ctrl); // 居中显示

    button_back();
}
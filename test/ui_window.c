#include "ui_window.h"
#include "lv_font_source_han_sans_bold.h"

lv_obj_t * img3 = NULL;                     /**< 临时图片对象 */

// ------------------------ 窗帘页事件 ------------------------
 void window_event_li(lv_event_t * e)
{
    animal_screen_exit();
    ui_load_page(window_page_li);
}

// -------------------- 窗帘控件 --------------------
static lv_obj_t * img_cl;
static int int_cl = 0;

// 滑块事件回调
void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider       = lv_event_get_target(e);
    lv_obj_t * slider_label = lv_event_get_user_data(e);

    int_cl = lv_slider_get_value(slider);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d%%", int_cl);
    lv_label_set_text(slider_label, buf);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_TOP_MID, 0, 0);

    if(img_cl) {
        lv_obj_set_size(img_cl, 6 * int_cl, 400);
        lv_img_set_src(img_cl, "S:/IOT/windows.png");
    }
}

// 页面创建函数
 void window_page_li(void)
{
    lv_obj_clean(lv_scr_act());
    //    animal_screen_exit();
    // 阳台背景图
    img3 = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img3, 0, 0);
    lv_img_set_src(img3, "S:/IOT/windows2.png");

    // 创建窗帘图片
    img_cl = lv_img_create(lv_scr_act());
    lv_img_set_src(img_cl, "S:/IOT/windows.png");
    lv_obj_set_pos(img_cl, 90, 30);
    lv_obj_set_size(img_cl, 50, 480); // 初始关闭

    // 创建滑块
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 30);

    // 创建滑块文本
    lv_obj_t * slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_color(slider_label, lv_color_white(), LV_PART_MAIN);

    // 滑块回调
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);

    button_back(); // 返回按钮
}

// 删除窗帘图片
void window_clear_cl(void)
{
    if(img_cl) {
        lv_obj_del(img_cl);
        img_cl = NULL;
    }
    if(img3) {
        lv_obj_del(img3);
        img3 = NULL;
    }
}
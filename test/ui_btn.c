#include "ui_btn.h"
#include "ui_home.h"
#include "lv_font_source_han_sans_bold.h"

void button_back(void)
{
    static lv_style_t style_transparent_btn6;
    lv_style_init(&style_transparent_btn6);
    lv_style_set_bg_opa(&style_transparent_btn6, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn6, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn6, LV_OPA_TRANSP);

    lv_obj_t * btn6 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn6, &style_transparent_btn6, 0);
    lv_obj_set_size(btn6, 50, 50);
    lv_obj_set_pos(btn6, 740, 420);
    lv_obj_add_event_cb(btn6, login_event_cb1, LV_EVENT_CLICKED, NULL);

    static lv_style_t style_pressed_glow6;
    lv_style_init(&style_pressed_glow6);
    lv_style_set_shadow_color(&style_pressed_glow6, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow6, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow6, 50);
    lv_obj_add_style(btn6, &style_pressed_glow6, LV_STATE_PRESSED);

    show_image("S:/IOT/back.png", 740, 420);
}

// ------------------------ 返回主页事件 ------------------------
static void login_event_cb1(lv_event_t * e)
{
    ui_load_page(lv_zhuce); // 返回登录页
}

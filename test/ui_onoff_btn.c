#include "ui_onoff_btn.h"
#include "lv_font_source_han_sans_bold.h"


// ------------------------ 开关页事件 ------------------------
 void on_off_event_li(lv_event_t * e)
{
    animal_screen_exit();
    ui_load_page(lv_li_on_off);
}

// ------------------------ 开关页面 ------------------------
void lv_li_on_off(void)
{
    // 背景图片
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img1, 0, 0);
    lv_img_set_src(img1, "S:/IOT/background4.png");

    button_back();
    on_off_page_li();
}



// -------------------- 开关控件 --------------------
static bool light_flag  = false;
static bool kong_flag   = false;
static bool camera_flag = false;
static bool feed_flag   = false;
void on_off_page_li(void)
{

    static lv_style_t style_transparent_btn8;
    lv_style_init(&style_transparent_btn8);
    lv_style_set_bg_opa(&style_transparent_btn8, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn8, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn8, LV_OPA_TRANSP);

    lv_obj_t * btn8 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn8, &style_transparent_btn8, 0);
    lv_obj_set_size(btn8, 230, 150);
    lv_obj_set_pos(btn8, 140, 110);

    // 创建图片
    lv_obj_t * img8 = lv_img_create(btn8);
    lv_img_set_src(img8, "S:/IOT/light_off.png");
    lv_obj_center(img8);

    static lv_style_t style_pressed_glow8;
    lv_style_init(&style_pressed_glow8);
    lv_style_set_shadow_color(&style_pressed_glow8, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow8, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow8, 80);
    lv_obj_add_style(btn8, &style_pressed_glow8, LV_STATE_PRESSED);

    // 标签
    lv_obj_t * label8 = lv_label_create(btn8);
    lv_label_set_text(label8, "灯光");
    lv_obj_align(label8, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_text_font(label8, &chinese_ziku, 0);
    lv_obj_set_style_text_color(label8, lv_color_hex(0xA0522D), LV_PART_MAIN);

    // 添加事件时，把图片指针作为用户数据传进去
    lv_obj_add_event_cb(btn8, light_event_li, LV_EVENT_CLICKED, img8);

    // lv_obj_t * img8 = lv_img_create(btn8);
    // lv_img_set_src(img8, "S:/IOT/cat2.png");
    // lv_obj_center(img8); // 图片在按钮内居中
    // lv_obj_align(img8, LV_ALIGN_CENTER, 0, 0); // 另一种写法

    // 空调按钮
    //  ---------------- 按键9 ----------------
    static lv_style_t style_transparent_btn9;
    lv_style_init(&style_transparent_btn9);
    lv_style_set_bg_opa(&style_transparent_btn9, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn9, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn9, LV_OPA_TRANSP);

    lv_obj_t * btn9 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn9, &style_transparent_btn9, 0);
    lv_obj_set_size(btn9, 230, 150);
    lv_obj_set_pos(btn9, 417, 110);

    // 创建图片
    lv_obj_t * img9 = lv_img_create(btn9);
    lv_img_set_src(img9, "S:/IOT/kong.png");
    lv_obj_center(img9);

    // 按下发光样式
    static lv_style_t style_pressed_glow9;
    lv_style_init(&style_pressed_glow9);
    lv_style_set_shadow_color(&style_pressed_glow9, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow9, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow9, 80);
    lv_obj_add_style(btn9, &style_pressed_glow9, LV_STATE_PRESSED);

    // 标签
    lv_obj_t * label9 = lv_label_create(btn9);
    lv_label_set_text(label9, "空调");
    lv_obj_align(label9, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_text_font(label9, &chinese_ziku, 0);
    lv_obj_set_style_text_color(label9, lv_color_hex(0xA0522D), LV_PART_MAIN);
    lv_obj_add_event_cb(btn9, kong_event_li, LV_EVENT_CLICKED, img9);

    // 监控按钮
    //  ---------------- 按键10 ----------------
    static lv_style_t style_transparent_btn10;
    lv_style_init(&style_transparent_btn10);
    lv_style_set_bg_opa(&style_transparent_btn10, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn10, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn10, LV_OPA_TRANSP);

    lv_obj_t * btn10 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn10, &style_transparent_btn10, 0);
    lv_obj_set_size(btn10, 230, 150);
    lv_obj_set_pos(btn10, 140, 290);

    // 创建图片
    lv_obj_t * img10 = lv_img_create(btn10);
    lv_img_set_src(img10, "S:/IOT/camera.png");
    lv_obj_center(img10);

    // 按下发光样式
    static lv_style_t style_pressed_glow10;
    lv_style_init(&style_pressed_glow10);
    lv_style_set_shadow_color(&style_pressed_glow10, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow10, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow10, 80);
    lv_obj_add_style(btn10, &style_pressed_glow10, LV_STATE_PRESSED);

    // 标签
    lv_obj_t * label10 = lv_label_create(btn10);
    lv_label_set_text(label10, "监控");
    lv_obj_align(label10, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_text_font(label10, &chinese_ziku, 0);
    lv_obj_set_style_text_color(label10, lv_color_hex(0xA0522D), LV_PART_MAIN);

    // 点击事件回调
    lv_obj_add_event_cb(btn10, camera_event_li, LV_EVENT_CLICKED, img10);
    //喂食器
    //  ---------------- 按键11 ----------------
    static lv_style_t style_transparent_btn11;
    lv_style_init(&style_transparent_btn11);
    lv_style_set_bg_opa(&style_transparent_btn11, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn11, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn11, LV_OPA_TRANSP);

    lv_obj_t * btn11 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn11, &style_transparent_btn11, 0);
    lv_obj_set_size(btn11, 230, 150);
    lv_obj_set_pos(btn11, 417, 290);

    // 创建图片
    lv_obj_t * img11 = lv_img_create(btn11);
    lv_img_set_src(img11, "S:/IOT/feed2.png"); 
    lv_obj_center(img11);

    // 按下发光样式
    static lv_style_t style_pressed_glow11;
    lv_style_init(&style_pressed_glow11);
    lv_style_set_shadow_color(&style_pressed_glow11, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow11, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow11, 80);
    lv_obj_add_style(btn11, &style_pressed_glow11, LV_STATE_PRESSED);

    // 标签
    lv_obj_t * label11 = lv_label_create(btn11);
    lv_label_set_text(label11, "喂食器");
    lv_obj_align(label11, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_text_font(label11, &chinese_ziku, 0);
    lv_obj_set_style_text_color(label11, lv_color_hex(0xA0522D), LV_PART_MAIN);

    // 点击事件回调
    lv_obj_add_event_cb(btn11, feed_event_li, LV_EVENT_CLICKED, img11);
    // 重置标志位
    light_flag  = false;
    kong_flag   = false;
    camera_flag = false;
    feed_flag   = false;
}


// ------------------------ 灯光事件 ------------------------
 void light_event_li(lv_event_t * e)
{

    lv_obj_t * img = lv_event_get_user_data(e); // 直接拿到图片对象

    light_flag = !light_flag;

    if(light_flag)
        lv_img_set_src(img, "S:/IOT/light_on.png");
    else
        lv_img_set_src(img, "S:/IOT/light_off.png");
}

 void kong_event_li(lv_event_t * e)
{

    lv_obj_t * img = lv_event_get_user_data(e); // 直接拿到图片对象

    kong_flag = !kong_flag;

    if(kong_flag)
        lv_img_set_src(img, "S:/IOT/kong2.png");

    else
        lv_img_set_src(img, "S:/IOT/kong.png");
}

 void camera_event_li(lv_event_t * e)
{

    lv_obj_t * img = lv_event_get_user_data(e); // 直接拿到图片对象

    camera_flag = !camera_flag;

    if(camera_flag)
        lv_img_set_src(img, "S:/IOT/camera2.png");

    else
        lv_img_set_src(img, "S:/IOT/camera.png");
}

 void feed_event_li(lv_event_t * e)
{

    lv_obj_t * img = lv_event_get_user_data(e); // 直接拿到图片对象

    feed_flag = !feed_flag;

    if(feed_flag)
        lv_img_set_src(img, "S:/IOT/feed.png");

    else
        lv_img_set_src(img, "S:/IOT/feed2.png");
}
#include "ui_home.h"
#include "ui_temp.h"
#include "ui_onoff_btn.h"
#include "lv_font_source_han_sans_bold.h"
#include "ui_info.h"
#include "ui_window.h"
#include "ui_game.h"

lv_obj_t * my_img_clean; /**< 背景图片句柄 */
#define NUM_IMAGES 4
int current_img        = 0;     /**< 当前显示的图片索引 */
lv_obj_t * scroll      = NULL;  /**< 滚动容器 */
lv_timer_t * img_timer = NULL;  /**< 图片切换定时器 */
bool auto_yes_no       = false; /**< 是否启用自动播放 */

// ------------------------ 主页 ------------------------
/**
 * @brief 创建界面
 * 包含4个透明按钮，点击可以触发注册事件
 */
void lv_zhuce(void)
{
    ui_init(); // 初始化LVGL（字体、样式等）
               // window_clear_cl();

    // --------- 创建四个透明按钮，每个按钮都有点击效果 ----------
    // 第一个按钮
    static lv_style_t style_transparent_btn1;
    lv_style_init(&style_transparent_btn1);
    lv_style_set_bg_opa(&style_transparent_btn1, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn1, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn1, LV_OPA_TRANSP);

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn1, &style_transparent_btn1, 0);
    lv_obj_set_size(btn1, 172, 64);
    lv_obj_set_pos(btn1, 596, 47);
    lv_obj_add_event_cb(btn1, photo_event_auto, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "自动播放");
    lv_obj_center(label1);
    lv_obj_set_style_text_font(label1, &chinese_ziku, 0);

    static lv_style_t style_pressed_glow1;
    lv_style_init(&style_pressed_glow1);
    lv_style_set_shadow_color(&style_pressed_glow1, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow1, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow1, 80);
    lv_obj_add_style(btn1, &style_pressed_glow1, LV_STATE_PRESSED);

    show_image("S:/IOT/dog1.png", 596, 60);

    // 第二个按钮
    static lv_style_t style_transparent_btn2;
    lv_style_init(&style_transparent_btn2);
    lv_style_set_bg_opa(&style_transparent_btn2, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn2, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn2, LV_OPA_TRANSP);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn2, &style_transparent_btn2, 0);
    lv_obj_set_size(btn2, 172, 64);
    lv_obj_set_pos(btn2, 596, 148);
    lv_obj_add_event_cb(btn2, photo_event_manual, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "手动播放");
    lv_obj_center(label2);
    lv_obj_set_style_text_font(label2, &chinese_ziku, 0);

    static lv_style_t style_pressed_glow2;
    lv_style_init(&style_pressed_glow2);
    lv_style_set_shadow_color(&style_pressed_glow2, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow2, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow2, 80);
    lv_obj_add_style(btn2, &style_pressed_glow2, LV_STATE_PRESSED);

    show_image("S:/IOT/dog2.png", 596, 160);

    // 第三个按钮
    static lv_style_t style_transparent_btn3;
    lv_style_init(&style_transparent_btn3);
    lv_style_set_bg_opa(&style_transparent_btn3, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn3, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn3, LV_OPA_TRANSP);

    lv_obj_t * btn3 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn3, &style_transparent_btn3, 0);
    lv_obj_set_size(btn3, 172, 64);
    lv_obj_set_pos(btn3, 596, 249);
    lv_obj_add_event_cb(btn3, temp_event_li, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label3 = lv_label_create(btn3);
    lv_label_set_text(label3, "温度调节");
    lv_obj_center(label3);
    lv_obj_set_style_text_font(label3, &chinese_ziku, 0);

    static lv_style_t style_pressed_glow3;
    lv_style_init(&style_pressed_glow3);
    lv_style_set_shadow_color(&style_pressed_glow3, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow3, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow3, 80);
    lv_obj_add_style(btn3, &style_pressed_glow3, LV_STATE_PRESSED);

    show_image("S:/IOT/cat1.png", 596, 260);

    // 第四个按钮
    static lv_style_t style_transparent_btn4;
    lv_style_init(&style_transparent_btn4);
    lv_style_set_bg_opa(&style_transparent_btn4, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn4, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn4, LV_OPA_TRANSP);

    lv_obj_t * btn4 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn4, &style_transparent_btn4, 0);
    lv_obj_set_size(btn4, 172, 64);
    lv_obj_set_pos(btn4, 596, 349);
    lv_obj_add_event_cb(btn4, on_off_event_li, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label4 = lv_label_create(btn4);
    lv_label_set_text(label4, "开关按钮");
    lv_obj_center(label4);
    lv_obj_set_style_text_font(label4, &chinese_ziku, 0);

    static lv_style_t style_pressed_glow4;
    lv_style_init(&style_pressed_glow4);
    lv_style_set_shadow_color(&style_pressed_glow4, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow4, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow4, 80);
    lv_obj_add_style(btn4, &style_pressed_glow4, LV_STATE_PRESSED);

    show_image("S:/IOT/cat2.png", 596, 360);

    // 个人信息控件

    show_image("S:/IOT/id.png", 800 - 50 - 10, 480 - 50 - 10);

    static lv_style_t style_transparent_btn5;
    lv_style_init(&style_transparent_btn5);
    lv_style_set_bg_opa(&style_transparent_btn5, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn5, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn5, LV_OPA_TRANSP);

    lv_obj_t * btn5 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn5, &style_transparent_btn5, 0);
    lv_obj_set_size(btn5, 50, 50);
    lv_obj_set_pos(btn5, 740, 420);
    lv_obj_add_event_cb(btn5, information_event_li, LV_EVENT_CLICKED, NULL);

    static lv_style_t style_pressed_glow5;
    lv_style_init(&style_pressed_glow5);
    lv_style_set_shadow_color(&style_pressed_glow5, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow5, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow5, 50);
    lv_obj_add_style(btn5, &style_pressed_glow5, LV_STATE_PRESSED);

    // 窗帘控件
    show_image("S:/IOT/window.png", 800 - 50 - 10 - 80, 480 - 50 - 10);//800 - 50 - 10 - 80,480 - 50 - 10

    static lv_style_t style_transparent_btn7;
    lv_style_init(&style_transparent_btn7);
    lv_style_set_bg_opa(&style_transparent_btn7, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn7, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn7, LV_OPA_TRANSP);

    lv_obj_t * btn7 = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn7, &style_transparent_btn7, 0);
    lv_obj_set_size(btn7, 50, 50);
    lv_obj_set_pos(btn7, 660, 420);
    lv_obj_add_event_cb(btn7, window_event_li, LV_EVENT_CLICKED, NULL);

    static lv_style_t style_pressed_glow7;
    lv_style_init(&style_pressed_glow7);
    lv_style_set_shadow_color(&style_pressed_glow7, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow7, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow7, 50);
    lv_obj_add_style(btn7, &style_pressed_glow7, LV_STATE_PRESSED);

    // 游戏控件

    show_image("S:/IOT/game.png", 580, 420);//800 - 50 - 10 - 80-80

    static lv_style_t style_transparent_btn_game;
    lv_style_init(&style_transparent_btn_game);
    lv_style_set_bg_opa(&style_transparent_btn_game, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_transparent_btn_game, LV_OPA_TRANSP);
    lv_style_set_shadow_opa(&style_transparent_btn_game, LV_OPA_TRANSP);

    lv_obj_t * btn_game = lv_btn_create(lv_scr_act());
    lv_obj_add_style(btn_game, &style_transparent_btn_game, 0);
    lv_obj_set_size(btn_game, 50, 50);
    lv_obj_set_pos(btn_game, 600, 420);
    lv_obj_add_event_cb(btn_game, game_event_manual, LV_EVENT_CLICKED, NULL);

    static lv_style_t style_pressed_glow_game;
    lv_style_init(&style_pressed_glow_game);
    lv_style_set_shadow_color(&style_pressed_glow_game, lv_color_hex(0x00BFFF));
    lv_style_set_shadow_opa(&style_pressed_glow_game, LV_OPA_70);
    lv_style_set_shadow_width(&style_pressed_glow_game, 50);
    lv_obj_add_style(btn7, &style_pressed_glow_game, LV_STATE_PRESSED);

    // 框内图片
    my_img_clean = show_image("S:/IOT/background3.png", 57, 83);
    // lv_obj_del(my_img_clean);
    // my_img_clean = NULL; // 防止野指针
    create_time_widget();
}

// ------------------------ 手动播放事件 ------------------------
void photo_event_manual(lv_event_t * e)
{
    auto_yes_no = false;

    if(my_img_clean != NULL) {
        lv_obj_del(my_img_clean);
        my_img_clean = NULL; // 防止野指针
    }

    animal_screen_exit();
    load_animal_smart_home_screen();
    //  animal_screen_exit();
    // ui_load_page(lv_scr_act());
}
// ------------------------ 自动播放事件 ------------------------
void photo_event_auto(lv_event_t * e)
{
    auto_yes_no = true;

    if(my_img_clean != NULL) {
        lv_obj_del(my_img_clean);
        my_img_clean = NULL; // 防止野指针
    }

    if(img_timer) {
        lv_timer_pause(img_timer); // 立即暂停定时器
        lv_timer_del(img_timer);   // 删除定时器
        img_timer = NULL;
    }

    if(scroll) {
        // 递归删除所有子对象
        lv_obj_clean(scroll);
        lv_obj_del(scroll);
        scroll = NULL;
    }

    animal_screen_exit();
    load_animal_smart_home_screen();
    //  animal_screen_exit();
    // ui_load_page(lv_scr_act());
}

// 页面退出或跳转前调用
void animal_screen_exit(void)
{
    // 删除定时器
    if(img_timer) {
        lv_timer_del(img_timer);
        img_timer = NULL;
    }

    // 删除滚动容器
    if(scroll) {
        lv_obj_del(scroll);
        scroll = NULL;
    }

    // 重置索引
    current_img = 0;
}

// -------------------- 加载动物智能家居页面 --------------------
extern const char * animal_images[] = {"S:/IOT/cat_1.png", "S:/IOT/cat_2.png", "S:/IOT/dog_1.png",
                                       "S:/IOT/dog_2.png"}; // 图片数组

void load_animal_smart_home_screen(void)
{
    // 防止重复创建
    if(scroll != NULL || img_timer != NULL) {
        // 已经在播放，直接返回
        printf("自动播放已在进行中，忽略重复点击。\n");
        return;
    }

    // 创建一个水平可滑动容器（用于多张图片）
    scroll = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scroll, 508, 293);
    lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_ROW);          // 水平排列
    lv_obj_set_scroll_dir(scroll, LV_DIR_HOR);               // 启用水平滑动
    lv_obj_set_scroll_snap_x(scroll, LV_SCROLL_SNAP_CENTER); // 吸附居中
    lv_obj_set_style_pad_all(scroll, 0, 0);                  // 去除内边距
    lv_obj_set_style_bg_opa(scroll, LV_OPA_TRANSP, 0);       // 背景透明

    lv_obj_set_pos(scroll, 57, 83);
    // 循环创建图片
    for(int i = 0; i < NUM_IMAGES; i++) {
        lv_obj_t * img = lv_img_create(scroll);
        lv_img_set_src(img, animal_images[i]); // 动物图片路径
        lv_obj_set_size(img, 508, 293);
        lv_obj_center(img);
    }

    if(auto_yes_no == true) {
        img_timer = lv_timer_create(switch_img_cb, 500, NULL);
    }
}

void switch_img_cb(lv_timer_t * timer)
{
    if(scroll == NULL) return; // scroll 已经被删除，直接返回

    // 计算水平偏移，让当前图片居中
    lv_coord_t scroll_x = current_img * 518;          // 假设每张图片宽 58
    lv_obj_scroll_to_x(scroll, scroll_x, LV_ANIM_ON); // 带动画滚动

    current_img = (current_img + 1) % NUM_IMAGES; // 循环索引
}

// ------------------------ 延迟跳转函数 ------------------------
/**
 * @brief 延迟跳转到注册页(未使用)
 */

void switch_to_zhuce(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    ui_load_page(lv_zhuce);
}


// ------------------------ 游戏事件 ------------------------
void game_event_manual(lv_event_t * e)
{

    if(my_img_clean != NULL) {
        lv_obj_del(my_img_clean);
        my_img_clean = NULL; // 防止野指针
    }

    animal_screen_exit();
    create_2048_page_and_load();
}
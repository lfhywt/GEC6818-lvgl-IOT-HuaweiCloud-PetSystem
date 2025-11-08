#include "uptime.h"
#include "lv_font_source_han_sans_bold.h"
#include "ui_window.h"
#include "ui_info.h"
#include "ui_onoff_btn.h"
#include "ui_temp.h"
#include "ui_time.h"
#include "ui_home.h"
#include "ui_manager.h"

// ------------------------ 全局变量 ------------------------
static lv_obj_t * bar;                      /**< 启动页进度条对象 */
static lv_timer_t * progress_timer = NULL;  /**< 启动画面进度条定时器 */
static int progress_value          = 0;     /**< 当前进度百分比 */
static bool load_done              = false; /**< 加载完成标志位 */
lv_timer_t * delay_timer;                   /**< 延时跳转定时器 */
static bool kaiji_finished = false;         /**< 启动页是否完成标记 */
lv_obj_t * label;                           /**< 百分比标签 */

// ------------------------ 启动界面（进度条+GIF） ------------------------
void lv_li_kaiji(void)
{
    // 背景图片
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img1, 0, 0);
    lv_img_set_src(img1, "S:/IOT/background.png");

    // 背景文字
    lv_obj_t * backgroud_word = lv_label_create(lv_scr_act());
    lv_label_set_text(backgroud_word, "正在初始化...");
    lv_obj_set_style_text_font(backgroud_word, &chinese_ziku, 0);
    lv_obj_center(backgroud_word);
    lv_obj_align(backgroud_word, LV_ALIGN_CENTER, 0, 160);

    // 创建进度条
    bar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(bar, 400, 20);
    lv_obj_set_pos(bar, 200, 300);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    // 进度条背景样式
    static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_radius(&style_bg, 10);
    lv_style_set_bg_opa(&style_bg, LV_OPA_30);
    lv_style_set_bg_color(&style_bg, lv_color_hex(0x808080));
    lv_obj_add_style(bar, &style_bg, LV_PART_MAIN);

    // 进度条前景样式（蓝色渐变）
    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_radius(&style_indic, 10);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);
    lv_style_set_bg_color(&style_indic, lv_color_hex(0x1E90FF));
    lv_style_set_bg_grad_color(&style_indic, lv_color_hex(0x00BFFF));
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    // 百分比标签
    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "0%");
    lv_obj_align_to(label, bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 初始化进度值
    progress_value = 0;

    // 创建进度条定时器（50ms刷新一次）
    progress_timer = lv_timer_create(progress_timer_cb, 50, NULL);

    // GIF动画
    lv_obj_t * gif1 = lv_gif_create(lv_scr_act());
    lv_obj_set_pos(gif1, 259, 191);
    lv_gif_set_src(gif1, "S:/IOT/sucai.gif");
}

// ------------------------ 进度条定时器回调 ------------------------
static void progress_timer_cb(lv_timer_t * timer)
{
    // LV_UNUSED(timer);

    if(kaiji_finished) {
        // 用户手动跳转了页面，直接删除定时器，不再处理
        if(progress_timer) {
            lv_timer_del(progress_timer);
            progress_timer = NULL;
        }
        return;
    }

    progress_value += 3; // 每次增加3%
    if(progress_value > 100) progress_value = 100;

    if(bar && label) {
        lv_bar_set_value(bar, progress_value, LV_ANIM_OFF);

        static char buf[8];
        sprintf(buf, "%d%%", progress_value);
        lv_label_set_text(label, buf);
    }
    // 当进度到100%时
    if(progress_value >= 100) {
        // 删除定时器，避免重复调用
        if(progress_timer) {
            lv_timer_del(progress_timer);
            progress_timer = NULL;
        }
        ui_load_page(lv_zhuce);
        // 延迟100ms后跳转注册页
        // delay_timer = lv_timer_create(switch_to_zhuce, 100, NULL);
        // lv_timer_set_repeat_count(delay_timer, 1); // 只执行一次
    }
}

// ------------------------ 图片导入函数 ------------------------
lv_obj_t * show_image(const char * img_path, lv_coord_t x, lv_coord_t y)
{
    // 创建图片对象
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img, x, y);     // 设置图片位置
    lv_img_set_src(img, img_path); // 设置图片路径

    // 如果想缩放图片，可以使用 lv_img_set_zoom()
    // lv_img_set_zoom(img, 256); // 256 = 100%，512 = 200%
    return img;
}

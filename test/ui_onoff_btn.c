#include "ui_onoff_btn.h"
#include "lv_font_source_han_sans_bold.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ui_home.h"
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#define SERVER_IP "192.168.171.60"
#define SERVER_PORT 10000
/* --- 在文件顶部（全局区）新增这些声明，放在 bool light_flag; 等同一位置 --- */
#include <stdatomic.h> // 如果没有可以用 volatile sig_atomic_t 代替
#include <signal.h>
#include "led.h"
pthread_mutex_t flags_mutex = PTHREAD_MUTEX_INITIALIZER;
/* === 新增：条件变量，用于唤醒上传线程立即发送 === */
pthread_cond_t need_send_cond = PTHREAD_COND_INITIALIZER;

/* 让上报线程知道需要立即发送一次最新状态（由按钮设置） */
volatile sig_atomic_t need_send_now = 0;

bool light_flag;
bool kong_flag;
bool camera_flag;
bool feed_flag;

int ctrl_threads_created           = 0; // 线程是否已创建
volatile sig_atomic_t exit_threads = 0; // 线程退出标志

static lv_obj_t * img_light  = NULL;
static lv_obj_t * img_kong   = NULL;
static lv_obj_t * img_camera = NULL;
static lv_obj_t * img_feed   = NULL;

extern int g_sock;
extern pthread_mutex_t g_sock_lock;

void on_off_page_exit(void)
{
    
    // 重置全局指针（关键！避免残留旧地址）
    img_light  = NULL;
    img_kong   = NULL;
    img_camera = NULL;
    img_feed   = NULL;

    // 2. 手动删除所有创建的UI对象（避免 lv_obj_clean 遗漏）
    if(img_light) lv_obj_del(img_light);
    if(img_kong) lv_obj_del(img_kong);
    if(img_camera) lv_obj_del(img_camera);
    if(img_feed) lv_obj_del(img_feed);

    exit_threads = 1;                     // 触发线程退出条件
    pthread_cond_signal(&need_send_cond); // 唤醒阻塞的线程
    usleep(100000);                       // 等待线程退出

    // 重置标志，允许下次重新创建线程
    ctrl_threads_created = 0;
    exit_threads         = 0;
}

void * led_thread(void * arg)
{
    while(!exit_threads) {  // 响应退出标志
        LED(&exit_threads);  // 传入退出标志，单次执行后返回
        usleep(100000);      // 短暂休眠，降低CPU占用
    }
    return NULL;
}

/* -------------------- 改写后的 report_status_thread -------------------- */
void * report_status_thread(void * arg)
{
    (void)arg;
    char json_buf[256];

    while(!exit_threads) {
        /* === 每秒自动触发 + 支持立即唤醒 === */
        struct timeval now;
        gettimeofday(&now, NULL);

        struct timespec ts;
        ts.tv_sec  = now.tv_sec + 1; // 每秒超时
        ts.tv_nsec = now.tv_usec * 1000;

        pthread_mutex_lock(&flags_mutex);
        pthread_cond_timedwait(&need_send_cond, &flags_mutex, &ts);
        int lf = light_flag;
        int kf = kong_flag;
        int cf = camera_flag;
        int ff = feed_flag;
        pthread_mutex_unlock(&flags_mutex);

        snprintf(json_buf, sizeof(json_buf), "{\"light\":%d,\"kong\":%d,\"camera\":%d,\"feed\":%d}\n", lf, kf, cf, ff);

        pthread_mutex_lock(&g_sock_lock);
        if(g_sock >= 0) {
            ssize_t n = send(g_sock, json_buf, strlen(json_buf), 0);
            if(n < 0 && errno != EINTR) {
                perror("report_status_thread send");
                close(g_sock);
                g_sock = -1;
            }
        }
        pthread_mutex_unlock(&g_sock_lock);
    }
    return NULL;
}
// ------------------------ 接收控制消息线程 ------------------------
void * recv_ctrl_thread(void * arg)
{
    char buf[512];

    while(!exit_threads) {
        if(g_sock < 0) {
            sleep(1);
            continue;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(g_sock, &readfds);

        struct timeval tv;
        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        int ret = select(g_sock + 1, &readfds, NULL, NULL, &tv);
        if(ret < 0) {
            // select 被中断时重新进入循环
            if(errno == EINTR) continue;
            perror("select");
            sleep(1);
            continue;
        } else if(ret == 0) {
            // 超时无数据，不阻塞
            continue;
        }

        memset(buf, 0, sizeof(buf));

        // recv 加锁防止与发送线程冲突
        pthread_mutex_lock(&g_sock_lock);
        int len = recv(g_sock, buf, sizeof(buf) - 1, 0);
        pthread_mutex_unlock(&g_sock_lock);

        if(len <= 0) {
            printf("⚠️ 服务器断开连接，关闭socket并等待重连...\n");
            close(g_sock);
            g_sock = -1;
            sleep(2);
            continue;
        }

        buf[len] = '\0';
        printf("📩 收到控制消息: %s\n", buf);

        // ---- JSON解析部分，不动你的逻辑 ----
        int light = -1, kong = -1, camera = -1, feed = -1;
        char * p;
        if((p = strstr(buf, "\"light\"")) != NULL) sscanf(p, "\"light\"%*[^0-9]%d", &light);
        if((p = strstr(buf, "\"kong\"")) != NULL) sscanf(p, "\"kong\"%*[^0-9]%d", &kong);
        if((p = strstr(buf, "\"camera\"")) != NULL) sscanf(p, "\"camera\"%*[^0-9]%d", &camera);
        if((p = strcasestr(buf, "\"feed\"")) != NULL) sscanf(p, "\"feed\"%*[^0-9]%d", &feed);

        // ---- 更新标志位 ----
        if(light != -1) light_flag = (light == 1);
        if(kong != -1) kong_flag = (kong == 1);
        if(camera != -1) camera_flag = (camera == 1);
        if(feed != -1) feed_flag = (feed == 1);

        printf("✅ 状态更新 -> light=%d kong=%d camera=%d feed=%d\n", light_flag, kong_flag, camera_flag, feed_flag);

        pthread_cond_signal(&need_send_cond);                // ✅ 唤醒上报线程同步状态
        lv_async_call((lv_async_cb_t)update_ui_state, NULL); // ✅ 异步更新UI
        continue;
    }
    return NULL;
}

void update_ui_state(void)
{
    // 防止还没创建对象时崩溃
    if(!img_light || !img_kong || !img_camera || !img_feed) return;

    lv_img_set_src(img_light, light_flag ? "S:/IOT/light_on.png" : "S:/IOT/light_off.png");
    lv_img_set_src(img_kong, kong_flag ? "S:/IOT/kong2.png" : "S:/IOT/kong.png");
    lv_img_set_src(img_camera, camera_flag ? "S:/IOT/camera2.png" : "S:/IOT/camera.png");
    lv_img_set_src(img_feed, feed_flag ? "S:/IOT/feed.png" : "S:/IOT/feed2.png");
}

// ------------------------ 开关页事件 ------------------------
void on_off_event_li(lv_event_t * e)
{
    animal_screen_exit();
    ui_load_page(lv_li_on_off);
}

// ------------------------ 开关页面 ------------------------
void lv_li_on_off(void)
{
    
        // 新增：进入页面时先清理当前屏幕所有旧元素
    lv_obj_clean(lv_scr_act());
    // 背景图片
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_obj_set_pos(img1, 0, 0);
    lv_img_set_src(img1, "S:/IOT/background4.png");

    button_back();
    on_off_page_li();
}

// -------------------- 开关控件 --------------------

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
    // 喂食器
    //   ---------------- 按键11 ----------------
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

    /* ✅ 修复：页面初始化时强制触发一次立即上传 */
    pthread_mutex_lock(&flags_mutex);
    need_send_now = 1;
    pthread_mutex_unlock(&flags_mutex);

    // 强制第一次同步 UI 显示当前状态
    lv_async_call((lv_async_cb_t)update_ui_state, NULL);

    img_light  = img8;
    img_kong   = img9;
    img_camera = img10;
    img_feed   = img11;
    pthread_t tid_send, tid_recv, tid_led;
    // 启动状态上报线程
    // 启动状态上报线程前检查并重建连接
    // 启动线程前加锁检查，防止并发创建
    pthread_mutex_lock(&flags_mutex);
    if(ctrl_threads_created == 0) {
        // 重建连接逻辑...

        pthread_create(&tid_send, NULL, report_status_thread, NULL);
        pthread_detach(tid_send);

        pthread_create(&tid_recv, NULL, recv_ctrl_thread, NULL);
        pthread_detach(tid_recv);

        pthread_create(&tid_led, NULL, led_thread, NULL);
        pthread_detach(tid_led);

        ctrl_threads_created = 1;
        printf("控制界面线程创建成功\n");
    }
    pthread_mutex_unlock(&flags_mutex);
}

// ------------------------ 灯光事件 ------------------------
void light_event_li(lv_event_t * e)
{

    lv_obj_t * img_light = lv_event_get_user_data(e); // 直接拿到图片对象
    pthread_mutex_lock(&flags_mutex);
    light_flag    = !light_flag;
    need_send_now = 1;
    pthread_mutex_unlock(&flags_mutex);
    pthread_cond_signal(&need_send_cond); // ← 新增这一行
    if(light_flag)
        lv_img_set_src(img_light, "S:/IOT/light_on.png");
    else
        lv_img_set_src(img_light, "S:/IOT/light_off.png");
}

void kong_event_li(lv_event_t * e)
{

    lv_obj_t * img_kong = lv_event_get_user_data(e); // 直接拿到图片对象
    pthread_mutex_lock(&flags_mutex);
    kong_flag     = !kong_flag;
    need_send_now = 1;
    pthread_mutex_unlock(&flags_mutex);
    pthread_cond_signal(&need_send_cond); // ← 新增这一行
    if(kong_flag)
        lv_img_set_src(img_kong, "S:/IOT/kong2.png");

    else
        lv_img_set_src(img_kong, "S:/IOT/kong.png");
}

void camera_event_li(lv_event_t * e)
{

    lv_obj_t * img_camera = lv_event_get_user_data(e); // 直接拿到图片对象
    pthread_mutex_lock(&flags_mutex);
    camera_flag   = !camera_flag;
    need_send_now = 1;
    pthread_mutex_unlock(&flags_mutex);
    pthread_cond_signal(&need_send_cond); // ← 新增这一行
    if(camera_flag)
        lv_img_set_src(img_camera, "S:/IOT/camera2.png");

    else
        lv_img_set_src(img_camera, "S:/IOT/camera.png");
}

void feed_event_li(lv_event_t * e)
{
    lv_obj_t * img_feed = lv_event_get_user_data(e); // 直接拿到图片对象
    pthread_mutex_lock(&flags_mutex);
    feed_flag     = !feed_flag;
    need_send_now = 1;
    pthread_mutex_unlock(&flags_mutex);
    pthread_cond_signal(&need_send_cond); // 唤醒发送线程

    if(feed_flag)
        lv_img_set_src(img_feed, "S:/IOT/feed.png");
    else
        lv_img_set_src(img_feed, "S:/IOT/feed2.png");
}

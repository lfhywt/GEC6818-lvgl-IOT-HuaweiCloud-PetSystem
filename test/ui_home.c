#include "ui_home.h"
#include "ui_temp.h"
#include "ui_onoff_btn.h"
#include "lv_font_source_han_sans_bold.h"
#include "ui_info.h"
#include "ui_window.h"
#include "ui_game.h"
#include <pthread.h>
#include <stdbool.h>

#define MAX_IMAGES 20
#define SAVE_DIR "/IOT/photo/"
#define CTRL_PORT 10000
#define IMG_PORT 10001
int thread_created = 0;

lv_obj_t * my_img_clean; /**< 背景图片句柄 */
// #define NUM_IMAGES 4
int NUM_IMAGES         = 0;
int current_img        = 0;     /**< 当前显示的图片索引 */
lv_obj_t * scroll      = NULL;  /**< 滚动容器 */
lv_timer_t * img_timer = NULL;  /**< 图片切换定时器 */
bool auto_yes_no       = false; /**< 是否启用自动播放 */
char * animal_images[MAX_IMAGES];
pthread_mutex_t img_mutex     = PTHREAD_MUTEX_INITIALIZER; // 线程锁
pthread_mutex_t file_op_mutex = PTHREAD_MUTEX_INITIALIZER;
char pending_delete_file[128] = {0};
bool has_delete_request       = false;
int g_sock                    = -1;
pthread_mutex_t g_sock_lock   = PTHREAD_MUTEX_INITIALIZER;
extern int ctrl_threads_created; // 引用 ui_onoff_btn.c 中的全局变量，确保不会叠加线程

void start_network_thread(void)
{
    pthread_t tid;
    // 启动 connect_server_thread 子线程执行实际连接操作
    if(pthread_create(&tid, NULL, connect_server_thread, NULL) != 0) {
        perror("创建连接线程失败");
    } else {
        pthread_detach(tid); // 自动回收线程资源，避免内存泄漏
    }
}

void * connect_server_thread(void * arg)
{
    struct sockaddr_in servaddr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket 创建失败");
        return NULL;
    }

    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(CTRL_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 执行阻塞连接（在子线程中，不影响UI）
    if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("连接服务器失败");
        close(sock);
        pthread_mutex_lock(&g_sock_lock);
        g_sock = -1; // 更新连接状态
        pthread_mutex_unlock(&g_sock_lock);
        return NULL;
    }

    // 连接成功，更新全局socket
    printf("✅ 已连接服务器 %s:%d\n", SERVER_IP, CTRL_PORT);
    pthread_mutex_lock(&g_sock_lock);
    if(g_sock >= 0) close(g_sock); // 关闭旧连接
    g_sock = sock;
    pthread_mutex_unlock(&g_sock_lock);

    // 启动上报线程
    pthread_t tid;
    pthread_create(&tid, NULL, report_status_thread, NULL);
    pthread_detach(tid);

    return NULL;
}

// 安全措施
void check_pending_file_ops(void)
{
    pthread_mutex_lock(&file_op_mutex);
    if(has_delete_request) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s%s", SAVE_DIR, pending_delete_file);
        printf("安全删除图片: %s\n", filepath);
        remove(filepath);
        has_delete_request = false;
        refresh_image_screen(); // 刷新界面
    }
    pthread_mutex_unlock(&file_op_mutex);
}

// ------------------------ 轮播图片子线程 ------------------------
void * recv_client_info(void * arg)
{
    pthread_detach(pthread_self());

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv;
    serv.sin_family      = AF_INET;
    serv.sin_port        = htons(IMG_PORT);
    serv.sin_addr.s_addr = inet_addr(SERVER_IP);

    if(connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        printf("连接服务器失败\n");
        perror("");
        return NULL;
    }
    printf("已连接服务器，等待图片数据...\n");

    char cmd[16];
    while(1) {
        int n = recv(sock, cmd, sizeof(cmd), 0);
        if(n <= 0) break;

        // ========================= 添加图片 =========================
        if(strncmp(cmd, "IMG_ADD", 7) == 0) {
            char filename[128];
            recv(sock, filename, sizeof(filename), 0);
            filename[sizeof(filename) - 1] = '\0';

            int filesize = 0;
            recv(sock, &filesize, sizeof(filesize), 0);

            // 临时文件路径（防止LVGL读到半截文件）
            char tmp_path[256];
            snprintf(tmp_path, sizeof(tmp_path), "%s.tmp_%s", SAVE_DIR, filename);

            FILE * fp = fopen(tmp_path, "wb");
            if(!fp) {
                perror("打开临时文件失败");
                continue;
            }

            char buf[1024];
            int received = 0;
            while(received < filesize) {
                int len = recv(sock, buf, sizeof(buf), 0);
                if(len <= 0) break;
                fwrite(buf, 1, len, fp);
                received += len;
            }
            fclose(fp);

            // 写完后重命名为正式文件（原子操作，不会被LVGL读到一半）
            char final_path[256];
            snprintf(final_path, sizeof(final_path), "%s%s", SAVE_DIR, filename);
            rename(tmp_path, final_path);

            printf("收到图片: %s (%d 字节)\n", final_path, filesize);

            // 上锁，防止 LVGL 同时访问目录
            pthread_mutex_lock(&img_mutex);
            lv_async_call((lv_async_cb_t)refresh_image_screen, NULL);
            pthread_mutex_unlock(&img_mutex);
        }

        // ========================= 删除图片 =========================
        else if(strncmp(cmd, "IMG_DEL", 7) == 0) {
            int name_len;
            recv(sock, &name_len, sizeof(name_len), 0);
            char filename[128];
            recv(sock, filename, name_len, 0);
            filename[name_len] = '\0';

            pthread_mutex_lock(&file_op_mutex);
            strcpy(pending_delete_file, filename);
            has_delete_request = true; // 仅设置标志，不刷新UI
            pthread_mutex_unlock(&file_op_mutex);

            printf("收到删除请求: %s\n", filename);

            // ✅ 异步让主线程执行安全删除
            lv_async_call((lv_async_cb_t)check_pending_file_ops, NULL);
        }
    }

    close(sock);
    return NULL;
}

void refresh_image_screen(void)
{

    if(lv_disp_get_default() == NULL) {
        printf("LVGL尚未初始化，跳过刷新。\n");
        return;
    }

    pthread_mutex_lock(&img_mutex);

    if(scroll) {
        lv_obj_del(scroll);
        scroll = NULL;
    }

    for(int i = 0; i < NUM_IMAGES; i++) {
        if(animal_images[i]) {
            free(animal_images[i]);
            animal_images[i] = NULL;
        }
    }
    NUM_IMAGES = 0; // 防止旧值残留

    printf("正在打开目录: %s\n", SAVE_DIR);
    DIR * dir = opendir(SAVE_DIR);
    if(!dir) {
        printf("无法打开目录: %s\n", SAVE_DIR);
        pthread_mutex_unlock(&img_mutex); // ✅ 记得解锁
        return;
    }

    struct dirent * entry;
    NUM_IMAGES = 0;

    while((entry = readdir(dir)) != NULL && NUM_IMAGES < MAX_IMAGES) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        if(strstr(entry->d_name, ".png") || strstr(entry->d_name, ".jpg")) {
            char lvgl_path[256];
            snprintf(lvgl_path, sizeof(lvgl_path), "S:/IOT/photo/%s", entry->d_name);
            animal_images[NUM_IMAGES++] = strdup(lvgl_path);
            printf("检测到图片: %s (LVGL路径)\n", lvgl_path);
        }
    }

    closedir(dir);
    if(NUM_IMAGES == 0) {
        printf("未检测到任何图片。\n");
        pthread_mutex_unlock(&img_mutex);
        return;
    }

    load_animal_smart_home_screen();

    pthread_mutex_unlock(&img_mutex); // 🔓
}
// ------------------------ 主页 ------------------------
/**
 * @brief 创建界面
 * 包含4个透明按钮，点击可以触发注册事件
 */
void lv_zhuce(void)
{
        // 新增：清理旧元素
    lv_obj_clean(lv_scr_act());
    
    ui_init(); // 初始化LVGL（字体、样式等）
    pthread_mutex_lock(&g_sock_lock);
    if(g_sock < 0) {
        start_network_thread(); // 调用此处，触发 connect_server_thread
    }
    pthread_mutex_unlock(&g_sock_lock);

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
    // show_image("/IOT/dog1.png", 596, 60);

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
    show_image("S:/IOT/window.png", 800 - 50 - 10 - 80, 480 - 50 - 10); // 800 - 50 - 10 - 80,480 - 50 - 10

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

    show_image("S:/IOT/game.png", 580, 420); // 800 - 50 - 10 - 80-80

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

    refresh_image_screen();

    // ------------------------ 创建线程 ------------------------
    // 创建新的子线程用来接收客户端的数据
    if(thread_created == 0) { // 还没创建过
        pthread_t tid;
        int ret = pthread_create(&tid, NULL, recv_client_info, NULL);
        if(ret != 0) {
            printf("pthread_create fail\n");
            return -1;
        }
        pthread_detach(tid); // 可选，让线程自动回收
        thread_created = 1;  // ✅ 设置为已创建
        printf("✅ recv_client_info 线程已创建\n");
    } else {
        printf("⚠️ 线程已创建，跳过重复创建\n");
    }
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
// extern const char * animal_images[] = {"S:/IOT/cat_1.png", "S:/IOT/cat_2.png", "S:/IOT/dog_1.png",
//                                        "S:/IOT/dog_2.png"}; // 图片数组

void load_animal_smart_home_screen(void)
{
    if(scroll != NULL || img_timer != NULL) {
        printf("自动播放已在进行中，忽略重复点击。\n");
        return;
    }

    scroll = lv_obj_create(lv_scr_act());
    lv_obj_set_size(scroll, 508, 293);
    lv_obj_set_flex_flow(scroll, LV_FLEX_FLOW_ROW);
    lv_obj_set_scroll_dir(scroll, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(scroll, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_style_pad_all(scroll, 0, 0);
    lv_obj_set_style_bg_opa(scroll, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(scroll, 57, 83);

    for(int i = 0; i < NUM_IMAGES; i++) {
        lv_obj_t * img = lv_img_create(scroll);
        lv_img_set_src(img, animal_images[i]);
        lv_obj_set_size(img, 508, 293);
        lv_obj_center(img);
    }

    if(auto_yes_no == true) {
        img_timer = lv_timer_create(switch_img_cb, 2000, NULL);
    }

    printf("页面加载完成，共加载 %d 张图片。\n", NUM_IMAGES);
}

void switch_img_cb(lv_timer_t * timer)
{
    if(scroll == NULL) return;

    pthread_mutex_lock(&file_op_mutex);
    bool need_delete = has_delete_request;
    pthread_mutex_unlock(&file_op_mutex);

    if(need_delete) {
        // ✅ 改成异步触发，让主线程执行删除和刷新
        lv_async_call((lv_async_cb_t)check_pending_file_ops, NULL);
        return;
    }

    // 自动播放逻辑
    lv_coord_t scroll_x = current_img * 518;
    lv_obj_scroll_to_x(scroll, scroll_x, LV_ANIM_ON);
    current_img = (current_img + 1) % NUM_IMAGES;
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

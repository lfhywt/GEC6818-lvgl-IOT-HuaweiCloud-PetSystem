#include "../lvgl/lvgl.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "ui_btn.h"


extern lv_font_t chinese_ziku; // 项目字体

#define GRID_N 4
#define TILE_EMPTY 0

static lv_obj_t * page_root;
static lv_obj_t * grid_cont;
static lv_obj_t * score_label;
static lv_obj_t * best_label;
static lv_obj_t * msg_label;

static int board[GRID_N][GRID_N];
static int score = 0;
static int best_score = 0;

// ui参数 800x400
static const int SCREEN_W = 800;
static const int SCREEN_H = 400;
static int grid_w, grid_h, cell_w, cell_h, grid_x, grid_y;

static lv_coord_t drag_start_x = -1;
static lv_coord_t drag_start_y = -1;

// ---------------- 辅助函数 ----------------
static int max_int(int a, int b) { return a > b ? a : b; }

static void update_best() {
    if(score > best_score) best_score = score;
    if(best_label) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Best: %d", best_score);
        lv_label_set_text(best_label, buf);
    }
}

static const lv_color_t tile_color_for_value(int v) {
    switch(v) {
        case 0: return lv_color_hex(0xEEE4DA);
        case 2: return lv_color_hex(0xEEE4DA);
        case 4: return lv_color_hex(0xEDE0C8);
        case 8: return lv_color_hex(0xF2B179);
        case 16: return lv_color_hex(0xF59563);
        case 32: return lv_color_hex(0xF67C5F);
        case 64: return lv_color_hex(0xF65E3B);
        case 128: return lv_color_hex(0xEDCF72);
        case 256: return lv_color_hex(0xEDCC61);
        case 512: return lv_color_hex(0xEDC850);
        case 1024: return lv_color_hex(0xEDC53F);
        case 2048: return lv_color_hex(0xEDC22E);
        default: return lv_color_hex(0x3C3A32);
    }
}

static lv_color_t text_color_for_value(int v) {
    if(v == 2 || v == 4) return lv_color_hex(0x776E65);
    return lv_color_hex(0xF9F6F2);
}

// ---------------- 游戏逻辑 ----------------
static void spawn_random_tile() {
    int empties = 0;
    for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++) if(board[i][j]==TILE_EMPTY) empties++;
    if(empties == 0) return;
    int target = rand() % empties;
    for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++) {
        if(board[i][j] == TILE_EMPTY) {
            if(target==0) {
                board[i][j] = (rand()%10==0) ? 4 : 2;
                return;
            }
            target--;
        }
    }
}

static void reset_board() {
    for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++) board[i][j]=TILE_EMPTY;
    score = 0;
    spawn_random_tile();
    spawn_random_tile();
}

static void rotate_board(int k) {
    k = (k%4+4)%4;
    for(int t=0;t<k;t++){
        int tmp[GRID_N][GRID_N];
        for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++) tmp[j][GRID_N-1-i] = board[i][j];
        for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++) board[i][j]=tmp[i][j];
    }
}

static bool move_left_once() {
    bool moved = false;
    for(int i=0;i<GRID_N;i++){
        int target = 0;
        for(int j=0;j<GRID_N;j++){
            if(board[i][j] != TILE_EMPTY){
                int val = board[i][j];
                board[i][j] = TILE_EMPTY;
                if(board[i][target] == TILE_EMPTY){
                    board[i][target] = val;
                } else if(board[i][target] == val){
                    board[i][target] = val * 2;
                    score += board[i][target];
                    target++;
                } else {
                    target++;
                    board[i][target] = val;
                }
                if(j != target) moved = true;
            }
        }
    }
    return moved;
}

static bool move_direction(int dir) {
    // dir: 0=left,1=up,2=right,3=down
    // 正确的旋转映射以保证移动方向一致：
    // left -> 0, up -> 3, right -> 2, down -> 1
    int rotate_times = 0;
    if(dir==0) rotate_times = 0;      // 左
    else if(dir==1) rotate_times = 3; // 上  <-- 修正
    else if(dir==2) rotate_times = 2; // 右
    else if(dir==3) rotate_times = 1; // 下  <-- 修正

    rotate_board(rotate_times);
    bool moved = move_left_once();
    rotate_board((4-rotate_times)%4);
    if(moved) spawn_random_tile();
    return moved;
}

static bool any_moves_available(){
    for(int i=0;i<GRID_N;i++) for(int j=0;j<GRID_N;j++){
        if(board[i][j]==TILE_EMPTY) return true;
        if(j<GRID_N-1 && board[i][j]==board[i][j+1]) return true;
        if(i<GRID_N-1 && board[i][j]==board[i+1][j]) return true;
    }
    return false;
}

// ---------------- UI刷新 ----------------
static void update_ui_tiles() {
    lv_obj_clean(grid_cont);
    for(int i=0;i<GRID_N;i++){
        for(int j=0;j<GRID_N;j++){
            lv_obj_t * cell = lv_obj_create(grid_cont);
            lv_obj_set_size(cell, cell_w, cell_h);
            int x = j * cell_w;
            int y = i * cell_h;
            lv_obj_set_pos(cell, x, y);
            lv_obj_clear_flag(cell, LV_OBJ_FLAG_CLICKABLE);

            int val = board[i][j];
            lv_obj_set_style_bg_color(cell, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_radius(cell, 8, LV_PART_MAIN);
            lv_obj_set_style_border_width(cell, 0, LV_PART_MAIN);

            if(val != TILE_EMPTY){
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", val);
                lv_obj_t * lbl = lv_label_create(cell);
                lv_label_set_text(lbl, buf);
                lv_obj_set_style_text_font(lbl, &chinese_ziku, 0);
                lv_obj_set_style_text_color(lbl, text_color_for_value(val), LV_PART_MAIN);
                lv_obj_center(lbl);
            }
        }
    }

    char s[64];
    snprintf(s, sizeof(s), "Score: %d", score);
    lv_label_set_text(score_label, s);
    update_best();
}

static void show_message(const char * txt) {
    if(msg_label) {
        lv_label_set_text(msg_label, txt);
        lv_obj_clear_flag(msg_label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void do_move_and_refresh(int dir) {
    if(move_direction(dir)) {
        update_ui_tiles();
        if(!any_moves_available()) {
            show_message("Game Over!");
        } else if(score >= 2048) {
            show_message("恭喜达到2048！");
        }
    }
}

static void btn_new_game_cb(lv_event_t * e) {
    LV_UNUSED(e);
    reset_board();
    update_ui_tiles();
    if(msg_label) lv_obj_add_flag(msg_label, LV_OBJ_FLAG_HIDDEN);
}

// ---------------- 手势事件修复 ----------------
static void root_pressing_event(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_PRESSED) {
        lv_indev_t * indev = lv_indev_get_act();
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        drag_start_x = p.x;
        drag_start_y = p.y;
    } 
    else if(code == LV_EVENT_RELEASED) {
        lv_indev_t * indev = lv_indev_get_act();
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        if(drag_start_x < 0) return;
        int dx = p.x - drag_start_x;
        int dy = p.y - drag_start_y;
        int adx = abs(dx), ady = abs(dy);
        const int SWIPE_THRESHOLD = 30;
        if(adx >= SWIPE_THRESHOLD || ady >= SWIPE_THRESHOLD) {
            if(adx > ady) {
                if(dx > 0) do_move_and_refresh(2); // 右
                else        do_move_and_refresh(0); // 左
            } else {
                if(dy < 0) do_move_and_refresh(1); // 上（修正方向）
                else        do_move_and_refresh(3); // 下
            }
        }
        drag_start_x = -1;
        drag_start_y = -1;
    }
}

// ---------------- 页面创建 ----------------
void create_2048_page(void)
{



    grid_w = 640;
    grid_h = 320;
    cell_w = grid_w / GRID_N;
    cell_h = grid_h / GRID_N;
    grid_x = (SCREEN_W - grid_w)/2;
    grid_y = (SCREEN_H - grid_h)/2 +20;

    page_root = lv_obj_create(NULL);
    lv_obj_set_size(page_root, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(page_root, lv_color_hex(0xFAF8EF), LV_PART_MAIN);
    lv_scr_load(page_root);

    // 顶部栏
    lv_obj_t * topbar = lv_obj_create(page_root);
    lv_obj_set_size(topbar, SCREEN_W, 60);
    lv_obj_align(topbar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_TRANSP, 0);

    score_label = lv_label_create(topbar);
    lv_label_set_text(score_label, "Score: 0");
    lv_obj_align(score_label, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_font(score_label, &chinese_ziku, 0);
    lv_obj_set_style_text_color(score_label, lv_color_hex(0x776E65), LV_PART_MAIN);

    best_label = lv_label_create(topbar);
    lv_label_set_text(best_label, "Best: 0");
    lv_obj_align(best_label, LV_ALIGN_RIGHT_MID, -150, 0);
    lv_obj_set_style_text_font(best_label, &chinese_ziku, 0);
    lv_obj_set_style_text_color(best_label, lv_color_hex(0x776E65), LV_PART_MAIN);

    lv_obj_t * new_btn = lv_btn_create(topbar);
    lv_obj_set_size(new_btn, 120, 40);
    lv_obj_align(new_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_event_cb(new_btn, btn_new_game_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * nb_label = lv_label_create(new_btn);
    lv_label_set_text(nb_label, "New Game");
    lv_obj_center(nb_label);

    msg_label = lv_label_create(page_root);
    lv_label_set_text(msg_label, "");
    lv_obj_align(msg_label, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_text_font(msg_label, &chinese_ziku, 0);
    lv_obj_set_style_text_color(msg_label, lv_color_hex(0xD9534F), LV_PART_MAIN);
    lv_obj_add_flag(msg_label, LV_OBJ_FLAG_HIDDEN);

    // 网格容器
    grid_cont = lv_obj_create(page_root);
    lv_obj_set_size(grid_cont, grid_w, grid_h);
    lv_obj_set_pos(grid_cont, grid_x, grid_y);
    lv_obj_set_style_bg_color(grid_cont, lv_color_hex(0xBBADA0), LV_PART_MAIN);
    lv_obj_set_style_radius(grid_cont, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_all(grid_cont, 8, LV_PART_MAIN);
    lv_obj_clear_flag(grid_cont, LV_OBJ_FLAG_CLICKABLE);

    // 注册手势事件
    lv_obj_add_event_cb(page_root, root_pressing_event, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(page_root, root_pressing_event, LV_EVENT_RELEASED, NULL);

    srand((unsigned)time(NULL));
    reset_board();
    update_ui_tiles();
}


void create_2048_page_and_load(void)
{
    lv_obj_clean(lv_scr_act());  // 清空当前屏幕

    create_2048_page();          // 创建新页面（含page_root）

    // 在 create_2048_page() 之后再加背景到 page_root
    lv_obj_t * img10 = lv_img_create(page_root);
    lv_obj_set_pos(img10, 0, 0);
    lv_img_set_src(img10, "S:/IOT/game1.png");
    lv_obj_move_background(img10);   // 确保在最底层显示

    button_back();  // 如果 button_back 是返回键，也放在当前页创建完后
}
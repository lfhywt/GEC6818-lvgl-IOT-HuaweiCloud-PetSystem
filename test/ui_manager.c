#include "ui_manager.h"
// ------------------------ 页面管理函数 ------------------------
/**
 * @brief 清空当前屏幕并加载指定页面
 * @param create_func 页面创建函数指针
 */
void ui_load_page(void (*create_func)(void))
{
    lv_obj_clean(lv_scr_act()); // 清空当前屏幕
    create_func();              // 创建新页面
}

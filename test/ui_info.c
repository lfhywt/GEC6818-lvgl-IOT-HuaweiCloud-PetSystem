#include "ui_info.h"
#include "lv_font_source_han_sans_bold.h"

// ------------------------ 管理员页事件 ------------------------
 void information_event_li(lv_event_t * e)
{
    animal_screen_exit();
    ui_load_page(information_page_li);
}

void information_page_li(void)
{

    button_back(); // 返回按钮

    // 从文件读取内容
    char * text = read_text_from_file("./info.txt");

    // 创建文本显示标签
    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);                        // 自动换行
    lv_obj_set_width(label1, 600);                                             // 控制显示宽度
    lv_obj_set_style_text_font(label1, &chinese_ziku, 0);                      // 使用字体
    lv_obj_set_style_text_color(label1, lv_color_hex(0xffffff), LV_PART_MAIN); // 黑色文字
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);              // 居中对齐

    // 设置文字内容
    if(text != NULL) {
        lv_label_set_text(label1, text);
        free(text);
    } else {
        lv_label_set_text(label1, "无法读取文件 ./info.txt");
    }

    // ⚠️ 一定要在 set_text() 之后再居中
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    // 滚动文字演示
    lv_obj_t * label2 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(label2, 200);
    lv_label_set_text(label2, "感谢使用本系统!仅供学习参考!");
    lv_obj_align(label2, LV_ALIGN_BOTTOM_MID, 0, -80);
    lv_obj_set_style_text_font(label2, &chinese_ziku, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0x007BFF), LV_PART_MAIN); // 蓝色
}

// static void back_event_handler(lv_event_t * e)
// {
//     lv_obj_t * btn = lv_event_get_target(e);
//     if(btn != NULL) {
//         // 创建并加载主界面
//         lv_obj_t * main_scr = lv_obj_create(NULL);
//         lv_obj_set_size(main_scr, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
//         init_main_screen_custom(main_scr);
//         lv_scr_load(main_scr);
//     }
// }



// ------------------------ 个人信息主页 ------------------------

// 从文件读取文本内容（返回 malloc 出的字符串，需要 free）
char * read_text_from_file(const char * path)
{
    FILE * fp = fopen(path, "r");
    if(!fp) {
        perror("打开文件失败"); //  打印系统错误
        printf("尝试打开路径: %s\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char * buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    printf("成功读取文件: %s\n", path);
    printf("文件内容:\n%s\n", buffer);

    return buffer;
}
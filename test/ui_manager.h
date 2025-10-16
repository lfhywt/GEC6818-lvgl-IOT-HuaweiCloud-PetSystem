#ifndef __TEMP_H_
#define __TEMP_H_

#include "../lvgl/lvgl.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>
void ui_load_page(void (*create_func)(void));

#endif
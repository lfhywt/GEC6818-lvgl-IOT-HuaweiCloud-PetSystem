#
# Makefile
#-L/home/gec/freetype-2.12.1/tmp/lib -I/home/gec/freetype-2.12.1/tmp/include/freetype2
CC = arm-linux-gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0   -I$(LVGL_DIR)/ -Wall  -std=gnu99
LDFLAGS ?= -lm 
BIN = demo
TESTSRC = $(wildcard ./test/*.c)
#TESTSRC = ./test/mywin.c  ./test/chinese_ziku.c  ./test/lv_font_source_han_sans_bold_20.c

#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
USRCODESRC = ${shell find $(LVGL_DIR)/mycode -name '*.c'}

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))
TESTOBJ = $(TESTSRC:.c=$(OBJEXT)) ##2. 添加.o文件
USRCODEOBJ = $(USRCODESRC:.c=$(OBJEXT))  #添加组件


SRCS = $(ASRCS) $(CSRCS) $(MAINSRC) $(USRCODESRC) #组件
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES


all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ) $(TESTOBJ) $(USRCODEOBJ) #组件
	$(CC) -o $(BIN) $(MAINOBJ) $(TESTOBJ) $(AOBJS) $(COBJS) $(LDFLAGS) $(USRCODEOBJ)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) $(TESTOBJ) $(USRCODEOBJ)


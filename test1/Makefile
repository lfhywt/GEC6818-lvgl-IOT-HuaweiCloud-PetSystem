CC = gcc
CFLAGS = -g -w -lrt -m64 -Wl,-z,relro,-z,now,-z,noexecstack -fno-strict-aliasing -fno-omit-frame-pointer -pipe -Wall -fPIC -MD -MP -fno-common -freg-struct-return  -fno-inline -fno-exceptions -Wfloat-equal -Wshadow -Wformat=2 -Wextra -rdynamic -Wl,-z,relro,-z,noexecstack -fstrength-reduce -fno-builtin -fsigned-char -ffunction-sections -fdata-sections -Wpointer-arith -Wcast-qual -Waggregate-return -Winline -Wunreachable-code -Wcast-align -Wundef -Wredundant-decls  -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs

CXXFLAGS = -O2 -g -Wall -fmessage-length=0 -lrt -m64 -Wl,-z,relro,-z,now,-z,noexecstack -fno-strict-aliasing -fno-omit-frame-pointer -pipe -Wall -fPIC -MD -MP -fno-common -freg-struct-return  -fno-inline -fno-exceptions -Wfloat-equal -Wshadow -Wformat=2 -Wextra -rdynamic -Wl,-z,relro,-z,noexecstack -fstack-protector-strong -fstrength-reduce -fno-builtin -fsigned-char -ffunction-sections -fdata-sections -Wpointer-arith -Wcast-qual -Waggregate-return -Winline -Wunreachable-code -Wcast-align -Wundef -Wredundant-decls  -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs

OBJS = AgentLiteDemo.o

HEADER_PATH = -I./include
LIB_PATH = -L./lib
SRC_PATH = ./src
# NTP客户端源文件路径（test目录下）
NTP_SRC_PATH = ./test1

LIBS = $(LIB_PATH) -lpaho-mqtt3as $(LIB_PATH) -lHWMQTT -lpthread

ifeq ($(OS), Windows_NT)
	TARGET = MQTT_Demo.exe
else 
	TARGET = MQTT_Demo.o
endif

# 原有MQTT目标
$(TARGET):	$(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

AgentLiteDemo.o: AgentLiteDemo.c
	$(CC) $(CFLAGS) -c AgentLiteDemo.c -o AgentLiteDemo.o $(HEADER_PATH)/agentlite/ $(HEADER_PATH)/service/ $(HEADER_PATH)/util/ $(HEADER_PATH)/third_party/cjson/

# 新增：NTP客户端编译目标（对应原指令gcc -o ntp_client main.c ntp_client.c）
ntp_client: $(NTP_SRC_PATH)/main.c $(NTP_SRC_PATH)/ntp_client.c
	$(CC) $(CFLAGS) -o $(NTP_SRC_PATH)/ntp_client $(NTP_SRC_PATH)/main.c $(NTP_SRC_PATH)/ntp_client.c -lpthread  # 增加-lpthread确保线程库链接

# 新增：编译后自动运行NTP客户端（对应原指令的&& ./ntp_client）
run_ntp: ntp_client
	$(NTP_SRC_PATH)/ntp_client

all:	$(TARGET)  # 保持默认目标不变，如需默认编译NTP可改为all: $(TARGET) ntp_client

clean:
	rm -f $(OBJS) $(TARGET) *.d ./server
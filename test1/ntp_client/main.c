#include <stdio.h>       // 标准输入输出(printf, fprintf)
#include <stdlib.h>      // 标准库函数(EXIT_FAILURE, EXIT_SUCCESS)
#include <time.h>        // 时间处理(localtime, strftime)
#include <sys/time.h>    // 时间类型(settimeofday, struct timeval)
#include "ntp_client.h"         // 包含NTP客户端头文件

int main() {
    // 从阿里云NTP服务器获取时间
    time_t ntp_time = get_ntp_time("182.92.12.11");
    
    // 检查获取时间是否失败
    if(ntp_time == (time_t)-1) {
        fprintf(stderr, "Failed to get NTP time\n"); // 打印错误信息到stderr
        return EXIT_FAILURE; // 退出程序并返回失败状态
    }
    
    // 准备时间结构
    // tv_sec: 秒数
    // tv_usec: 微秒数
    struct timeval tv = {
        .tv_sec = ntp_time,  // 设置秒数
        .tv_usec = 0         // 设置微秒数为0
    };
    
    // 设置系统时间(需要root权限)
    // &tv: 时间值结构指针
    // NULL: 时区信息(通常设为NULL)
    if(settimeofday(&tv, NULL) < 0) {
        perror("Failed to set system time (may need root privileges)");
        // 继续执行，即使没有设置系统时间权限
    }
    
    // 转换为本地时间结构
    // localtime: 将时间戳转换为本地时间(考虑时区)
    struct tm *tm_info = localtime(&ntp_time);
    if(tm_info == NULL) {
        perror("Localtime failed");
        return EXIT_FAILURE;
    }
    
    // 创建时间字符串缓冲区
    char time_buf[64];
    
    // 格式化时间
    // %Y: 4位年份
    // %m: 2位月份
    // %d: 2位日期
    // %H: 24小时制小时
    // %M: 分钟
    // %S: 秒
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // 打印格式化后的时间
    printf("NTP time: %s\n", time_buf);
    
    return EXIT_SUCCESS; // 退出程序并返回成功状态
}
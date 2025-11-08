#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define TEST_MAGIC 'x'                    //定义幻数

#define LED1 _IO(TEST_MAGIC, 0)              
#define LED2 _IO(TEST_MAGIC, 1)
#define LED3 _IO(TEST_MAGIC, 2)
#define LED4 _IO(TEST_MAGIC, 3) 

#define LED_ON  	0	//灯亮
#define LED_OFF		1   //灯灭

int main()
{
	//1.在程序执行前必须确保你的驱动已经安装驱动
	//2.访问驱动设备
	int fd = open("/dev/Led",O_RDWR);
	
	//3.控制设备状态
	while(1)
	{
		ioctl(fd,LED1,LED_ON);
		sleep(1);
		ioctl(fd,LED1,LED_OFF);
		sleep(1);
	}
	
	//4. 关闭设备文件
	close(fd);
	
	return 0;
}
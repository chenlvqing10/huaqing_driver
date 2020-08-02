#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/ioctl.h>
#include "cmd.h"
#include <sys/time.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>

int fd;
char ubuf[128] = "aaa"; //传递字符

//信号处理函数
void handle_fasync_fun(int signum)
{
	if(signum == SIGIO)
	{
		memset(ubuf,0,sizeof(ubuf));
		read(fd,ubuf,sizeof(ubuf));
		printf("ubuf = %s\n",ubuf);
	}
}
int main(int argc, const char *argv[])
{
	

	if((fd = open("/dev/myadc",O_RDWR))<0)
	{
		perror("open device");
		return -1;
	}

	//注册信号
	signal(SIGIO,handle_fasync_fun);
		
	//调用驱动的fasync函数,指定信号的进程
	int f_flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, f_flags | FASYNC);
	fcntl(fd,F_SETOWN,getpid());
	while(1)
	{
		printf("hello world\n");
		sleep(1);
	}
}


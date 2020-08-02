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
#include <poll.h>

#define   PRINT_ERROR(errmsg) do { perror(errmsg); return -1; }while(0)
#define   EVENT_SIZE  2
#define   ARRAY_SIZE(arr)  (sizeof(arr) / sizeof(arr[0]))

char ubuf[128] = "aaa"; //传递字符
int main(int argc, const char *argv[])
{
	int fd1,fd2,ret,i;
	struct pollfd pfd[EVENT_SIZE];//poll结构体数组
	//打开驱动
	fd1 = open("/dev/myadc",O_RDWR);
	if(fd1 == -1){
		perror("open /dev/myadc error");
		return -1;
	}
	printf("111111\n");
	fd2 = open("/dev/input/mouse0",O_RDONLY);
	if(fd2 == -1){
		perror("open /dev/input/mouse0 error");
		return -1;
	}

	//为poll结构体分配文件描述符和事件
	pfd[0].fd = fd1;
	pfd[0].events = POLLIN;
	pfd[1].fd = fd2;
	pfd[1].events = POLLIN;

	//循环等待事件触发，当超过timeout还没有事件触发时，就超时。
	while(1)
	{
		if((ret = poll(pfd,ARRAY_SIZE(pfd),-1) < 0))//返回被事件触发的事件(文件描述符)的个数
		{
			PRINT_ERROR("poll wait");
		}
		//printf("ret = %d\n",ret);
		//处理触发事件的文件描述符
		for(i=0;i<2;i++)
		{
			if(pfd[i].revents & POLLIN)//如果是输入事件
			{
				memset(ubuf,0,sizeof(ubuf));
				read(pfd[i].fd,ubuf,sizeof(ubuf));
				printf("ubuf%d = %s\n",pfd[i].fd,ubuf);
			}
		}
	}


	close(fd1);
	close(fd2);
}


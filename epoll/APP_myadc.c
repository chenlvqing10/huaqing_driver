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

#define   PRINT_ERROR(errmsg) do { perror(errmsg); return -1; }while(0)
#define   EVENT_SIZE  10
#define   ARRAY_SIZE(arr)  (sizeof(arr) / sizeof(arr[0]))
char ubuf[128] = "aaa"; //传递字符

int main(int argc, const char *argv[])
{
	int epfd,fd,i;
	struct epoll_event event;//要监听的事件
	struct epoll_event reevents[EVENT_SIZE];//返回的需要进行处理的事件数目  
	int ret;

	//创建epoll对象
	epfd  = epoll_create(EVENT_SIZE);
	if(epfd < 0)
	{
		PRINT_ERROR("create epoll error");
	}

	//将被监听的描述符添加到epoll句柄或从epool句柄中删除或者对监听事件进行修改。
	for(i=1;i<argc;i++)//命令行参数传参
	{
		if((fd = open(argv[i],O_RDWR))<0)
		{
			PRINT_ERROR("open event error");
		}

		//将需要监听的文件描述符加入到监听事件中
		event.events  = EPOLLIN;
		event.data.fd = fd;

		if(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event)<0)//讲监听事件添加到epoll对象中
		{
			PRINT_ERROR("add epoll ctl");
		}
	}


	//循环等待事件触发，当超过timeout还没有事件触发时，就超时。
	while(1)
	{
		if((ret = epoll_wait(epfd,reevents,ARRAY_SIZE(reevents),-1)) < 0)//返回被事件触发的事件(文件描述符)的个数
		{
			PRINT_ERROR("epoll wait");
		}
		//处理触发事件的文件描述符
		for(i=0;i<ret;i++)
		{
			if(reevents[i].events | EPOLLIN)//如果是输入事件
			{
				memset(ubuf,0,sizeof(ubuf));
				read(reevents[i].data.fd,ubuf,sizeof(ubuf));
				printf("ubuf%d = %s\n",reevents[i].data.fd,ubuf);
			}
		}
	}


	close(epfd);
}


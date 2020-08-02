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

char ubuf[128] = "aaa"; //传递字符

int main(int argc, const char *argv[])
{
	int fd1 = open("/dev/myadc",O_RDWR);
	int fd2 = open("/dev/input/mouse0",O_RDONLY);

	int fdmax = fd2 + 1;
	fd_set rfsd;//文件描述符表
	if(fd1==-1)
	{
		perror("open adc device");
		return -1;
	}

	if(fd2 == -1)
	{
		perror("open mouse device");
		return -1;
	}

	while(1)
	{
		//清空文件描述符表
		FD_ZERO(&rfsd);
		//将fd1,fd2放到表中
		FD_SET(fd1,&rfsd);
		FD_SET(fd1,&rfsd);
		int ret = select(fdmax,&rfsd,NULL,NULL,NULL);//循环等待write唤醒
		if(ret <0 )
		{
			perror("select error");
			return -1;
		}

		if(FD_ISSET(fd1,&rfsd))//如果fd1文件描述符被唤醒   ---硬件数据准备号了
		{
			memset(ubuf,0,sizeof(ubuf));
			read(fd1,ubuf,sizeof(ubuf));//读取fd1驱动数据到用户层
			printf("fd1 ubuf = %s\n",ubuf);

		}

		if(FD_ISSET(fd2,&rfsd))//如果fd2文件描述符被唤醒   ---硬件数据准备号了
		{
			memset(ubuf,0,sizeof(ubuf));
			read(fd2,ubuf,sizeof(ubuf));//读取fd2驱动数据到用户层
			printf("fd2 ubuf = %s\n",ubuf);
		}
	}


	close(fd1);
	close(fd2);
}


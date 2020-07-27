#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/ioctl.h>
#include "cmd.h"

#define RED 0
#define GREEN 1
#define blue  2

#define ON  0
#define OFF 1
//LED驱动 命令
//R  G   B
//0  1   2
//ON OFF
//0   1
bool is_ledhandle(char *buf)//不用指针操作，直接用buf[0],buf[1]
{
	int ret;
	if(((buf[0] =='0') || (buf[0] == '1')|| (buf[0] == '2')) && ((buf[1] == '0') || (buf[1] == '1')))
		ret = true;
	else
		ret = false;

	return ret;
}

char buf_toKenel[10] = {0}; //传递字符

int main(int argc, const char *argv[])
{
	int  fd;
	int  buf_len = 0;
	int data = 500;
	fd = open("/dev/myled",O_RDWR);
	if(fd == -1)
	{
		perror("open /dev/myled error");
		return -1;
	}
	printf("请输入两个由0和1的字符用于LED灯的控制(00 01 10 11 20 21):");
	scanf("%s",buf_toKenel);


	//进行合法性检测
	buf_len = strlen(buf_toKenel);
	while(((buf_len=strlen(buf_toKenel)) != 2) || (is_ledhandle(buf_toKenel)==false) )
	{
		printf("重新输入LED灯的控制命令(00 01 10 11 20 21):");
		fflush(stdin);
		scanf("%s",buf_toKenel);
	}
	printf("buf_toKenel = %s\n",buf_toKenel);

	int ret=write(fd,buf_toKenel,sizeof(buf_toKenel));//向内核发送LED灯的控制命令
	printf("ret = %d\n",ret);
	sleep(10);
	printf("在等待10s结束后你将看到跑马灯的现象\n");
	while(1)
	{

		ioctl(fd,RED_ON);
		sleep(1);
		ioctl(fd,RED_OFF);
		ioctl(fd,GREEN_ON);
		sleep(1);
		ioctl(fd,GREEN_OFF);
		ioctl(fd,BLUE_ON);
		sleep(1);
		ioctl(fd,BLUE_OFF);

		//传递数据
		ioctl(fd,ACCESS_DATA_W,&data);//write to kernel
		sleep(1);
		ioctl(fd,ACCESS_DATA_R,&data);//read from kernel
		printf("data fro kernel:%d\n",data);



		/*
		strcpy(buf_toKenel,"01");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		sleep(1);
		strcpy(buf_toKenel,"00");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		strcpy(buf_toKenel,"11");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		sleep(1);
		strcpy(buf_toKenel,"10");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		strcpy(buf_toKenel,"21");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		sleep(1);
		strcpy(buf_toKenel,"20");
		write(fd,buf_toKenel,sizeof(buf_toKenel));
		*/

	}
close(fd);

return 0;
}


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

char ubuf[128] = "aaa"; //传递字符

int main(int argc, const char *argv[])
{
	int fd = open("/dev/myadc",O_RDWR);
	if(fd==-1)
	{

		perror("open adc device");
		return -1;
	}

	printf("1111111\n");
	read(fd,ubuf,sizeof(ubuf));
	printf("1111111\n");
	printf("ubuf =%s\n",ubuf);


	/*
	   while(1)
	   {
	//read from kernel
	read(fd,ubuf,sizeof(ubuf));
	printf("adc voltage::%.2fV\n",atoi(ubuf)/1000.0);
	sleep(2);
	}
	return 0;
	*/

	close(fd);
}


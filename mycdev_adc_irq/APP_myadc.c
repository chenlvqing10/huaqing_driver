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

char ubuf[128] = "aaaaaaa"; //传递字符

int main(int argc, const char *argv[])
{
	printf("111111111111111111\n");
	int fd = open("/dev/farsight_adc_irq0",O_RDWR);
	printf("111111111111111111\n");
	if(fd==-1)
	{
	
		perror("open farsight_adc_irq device");
		return -1;
	}

	while(1)
	{
		//read from kernel
		read(fd,ubuf,sizeof(ubuf));
		printf("adc voltage::%.2fV\n",atoi(ubuf)/1000.0);
		sleep(2);
	}
	return 0;
}


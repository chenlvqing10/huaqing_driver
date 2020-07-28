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

char ubuf[128] = "aaaaaaa"; //传递字符

int main(int argc, const char *argv[])
{

	int fd_red,fd_green,fd_blue;
	fd_red    = open("/dev/mycdev0",O_RDWR);
	fd_green  = open("/dev/mycdev1",O_RDWR);
	fd_blue   = open("/dev/mycdev2",O_RDWR);

	if((fd_red==-1) || (fd_green==-1) || (fd_blue==-1))
	{
		perror("open /dev/mycdev0  /dev/mycdev1  /dev/mycdev2  error");
		return -1;
	}
	
     ubuf[0] = '1';
     while(1)
     {
         write(fd_red,ubuf,sizeof(ubuf));
         sleep(1);
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
         write(fd_red,ubuf,sizeof(ubuf));
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
         write(fd_green,ubuf,sizeof(ubuf));
         sleep(1);
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
         write(fd_green,ubuf,sizeof(ubuf));
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
         write(fd_blue,ubuf,sizeof(ubuf));
         sleep(1);
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
         write(fd_blue,ubuf,sizeof(ubuf));
         ubuf[0] = (ubuf[0]=='1')?'0':'1';
     }

	close(fd_red);
	close(fd_green);
	close(fd_blue);

	return 0;
}


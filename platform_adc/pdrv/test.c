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
 
 char ubuf[128] = "aaaaaaa"; //传递字符
 
 int main(int argc, const char *argv[])
 {
     int dgt_voltage = 0;
     float ang_voltage = (int)( dgt_voltage * 2 * ( (float) 1800 / 4096));
     int fd = open("/dev/farsight_adc_irq",O_RDWR);
     if(fd==-1)
     {
     
         perror("open farsight_adc_irq device");
         return -1;
     }
 
     while(1)
     {
         //read from kernel
         read(fd,&dgt_voltage,sizeof(dgt_voltage));
         ang_voltage  =  dgt_voltage / 4096.0 * 1.8;                                    
         printf("adc voltage::%.2fV\n",ang_voltage);
         sleep(1);
     }   
     return 0;
 }   
 


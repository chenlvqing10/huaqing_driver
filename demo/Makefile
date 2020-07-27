KERNELDIR := /lib/modules/$(shell uname -r)/build/  #内核源码目录
#KERNELDIR := /home/chenlvqing/Downloads/uboot移植__裁剪/kernel-3.4.39/   #内核源码目录
PWD       := $(shell pwd)                           #驱动所在目录

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules       #make -C dir:进入到dir目录下执行该目录下的Makefile文件  M=:程序当前路径 make modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean 		   #make -C dir:进入到dir目录下执行该目录下的Makefile文件  M=:程序当前路径 make modules
	sudo rm /lib/modules/5.3.0-62-generic/extra/*.ko
modules_install:
	sudo $(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
obj-m := demo.o






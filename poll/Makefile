MODNAME := myadc
MAJOR   := 244
APPNAME := APP_myadc.c
APPCOMPILER :=APP_myadc
KERNELDIR := /lib/modules/$(shell uname -r)/build/  #内核源码目录
#KERNELDIR := /home/chenlvqing/Downloads/uboot移植__裁剪/kernel-3.4.39/   #内核源码目录
PWD       := $(shell pwd)                           #驱动所在目录

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules       #make -C dir:进入到dir目录下执行该目录下的Makefile文件  M=:程序当前路径 make modules
	#arm-none-linux-gnueabi-gcc $(APPNAME) -o $(APPCOMPILER)
	#cp ./$(MODNAME).ko  $(APPCOMPILER)  ~/nfs/rootfs/
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean 		   #make -C dir:进入到dir目录下执行该目录下的Makefile文件  M=:程序当前路径 make modules
	#sudo rm /lib/modules/5.3.0-62-generic/extra/$(MODNAME).ko
	sudo rmmod $(MODNAME).ko
	sudo rm    /dev/$(MODNAME)

modules_install:
	sudo $(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	sudo insmod /lib/modules/5.3.0-62-generic/extra/$(MODNAME).ko
dev_install:
	sudo mknod /dev/$(MODNAME) c $(MAJOR) 0 

obj-m := $(MODNAME).o






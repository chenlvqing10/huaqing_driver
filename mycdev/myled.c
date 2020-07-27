#include "myled.h"


int myled_open(struct inode *__node, struct file *__fd)
{
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);

	//提取设备号
	//major = imajor(__node);
	//minor = iminor(__node);

	printk("major:%d  minor:%d\n",imajor(__node),iminor(__node));

	//通过file结构体得到一些信息
	printk("111:%d   222:%d\n",imajor(__fd->f_mapping->host),iminor(__fd->f_mapping->host));

	return 0;
}

int myled_close(struct inode *__node, struct file *__fd)
{
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

char kbuf_toUsr[128] = "hahahahahah  hahhahahah";
ssize_t myled_read(struct file * __fd, char __user *ubuf, size_t size, loff_t * __loff)
{
	int ret;
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);

	if(size > sizeof(kbuf_toUsr)){
		size = sizeof(kbuf_toUsr);
	}
	ret = copy_to_user(ubuf,kbuf_toUsr,size);
	if(ret)
	{
		printk("copy kbuf to user error\n");
		return -EIO;
	}
	return size;
}


int myled_kernel_hw_init(void)//LED内核与硬件交互的初始化
{
	int ret = 0;
	v_GPIOA  = ioremap(GPIOA,40);//40---10个寄存器地址
	v_GPIOB  = ioremap(GPIOB,40);
	v_GPIOE  = ioremap(GPIOE,40);

	if((v_GPIOA == NULL) || (v_GPIOE == NULL) || (v_GPIOE == NULL))
	{
		ret = -ENOMEM;
	}
	else
	{
		//设置寄存器为GIOP功能
		*(v_GPIOA + ALTFN1)  &= ~(0x3 << 24);//GPIO引脚为GPIO功能 red led
		*(v_GPIOE + ALTFN0)  &= ~(0x3 << 26);//GPIO引脚为GPIO功能 green led
		*(v_GPIOB + ALTFN0)  |=  (0x2 << 24);//GPIO引脚为GPIO功能 blue led
		//设置寄存器为输出功能
		*(v_GPIOA + OUTENB) |= (0x1 << 28);
		*(v_GPIOE + OUTENB) |= (0x1 << 13);
		*(v_GPIOB + OUTENB) |= (0x1 << 12);     

		//设置led熄灭状态  刚开始的白灯会有影响
		*(v_GPIOA + OUT) &= ~(0x1 << 28);
		*(v_GPIOE + OUT) &= ~(0x1 << 13);
		*(v_GPIOB + OUT) &= ~(0x1 << 12);
	}

	return ret;
}
void myled_setlamp(const char* kbuf)
{

	switch(kbuf[0])
	{
		case RED:
			(kbuf[1]=='1') ? (*(v_GPIOA + OUT) |=  (0x1 << 28)) : (*(v_GPIOA + OUT) &= ~(0x1 << 28));   //red led on/off
			break;
		case GREEN:
			(kbuf[1]=='1') ? (*(v_GPIOE + OUT) |=  (0x1 << 13)) : (*(v_GPIOE + OUT) &= ~(0x1 << 13));   //green led on/off
			break;
		case BLUE:
			(kbuf[1]=='1') ? (*(v_GPIOB + OUT) |=  (0x1 << 12)) : (*(v_GPIOB + OUT) &= ~(0x1 << 12));    //blue led on/off
			break;
		default:
			//设置led熄灭状态
			*(v_GPIOA + OUT) &= ~(0x1 << 28);
			*(v_GPIOE + OUT) &= ~(0x1 << 13);
			*(v_GPIOB + OUT) &= ~(0x1 << 12);
			break;
	}
}

ssize_t myled_write(struct file * __fd, const char __user * ubuf, size_t size, loff_t * __loff)
{
	int ret;
	printk("call write fun\n");

	if(size > sizeof(kbuf))//如果用户空间大小大于要拷贝的内核空间大小  采用内核空间大小 检查拷贝的用户空间大小参数
	{
		size = sizeof(kbuf);
	}
	ret = copy_from_user(kbuf,ubuf,size);
	if(ret)
	{
		printk("copy data from user error\n");
		return -EIO;//错误码
	}
	printk("kbuf =%s\n",kbuf);

	//通过输入的命令进行led灯的操作
	//myled_setlamp(kbuf);
	minor = iminor(__fd->f_mapping->host);
	if(minor == MINOR_RED_LED)
	{
		if(kbuf[0] == '1')
		{
			printk("I will put red led on\n");
			*(v_GPIOA + OUT) |=  (0x1 << 28);
		}
		else
		{
			printk("I will put red led off\n");
			*(v_GPIOA + OUT) &= ~(0x1 << 28);
		}
	}
	else if(minor == MINOR_GREEN_LED)
	{
		if(kbuf[0] == '1')
		{
			printk("I will put green led\n");
			*(v_GPIOE + OUT) |=  (0x1 << 13);
		}
		else
		{
			printk("I will put green led off\n");
		    *(v_GPIOE + OUT) &= ~(0x1 << 13);
		}
	}
	else if(minor == MINOR_BLUE_LED)
	{
		if(kbuf[0]=='1')
		{
			printk("I eill put blue led\n");
			*(v_GPIOB + OUT) |=  (0x1 << 12);
		}
		else
		{
			printk("I will put blue led off\n");
			*(v_GPIOB + OUT) &= ~(0x1 << 12);  
		}
	}

	return size;
}


long myled_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	int data = 111;
	char string_Buf[100] = "transfer data from kernel to user";
	int ret;

	switch(cmd){
		case RED_ON:
			(*(v_GPIOA + OUT) |=  (0x1 << 28));
			break;
		case RED_OFF:
			*(v_GPIOA + OUT) &= ~(0x1 << 28);
			break;
		case GREEN_ON:
			*(v_GPIOE + OUT) |=  (0x1 << 13);
			break;
		case GREEN_OFF:
			*(v_GPIOE + OUT) &= ~(0x1 << 13);
			break;
		case BLUE_ON:
			*(v_GPIOB + OUT) |=  (0x1 << 12);
			break;
		case BLUE_OFF:
			*(v_GPIOB + OUT) &= ~(0x1 << 12);
			break;
		case ACCESS_DATA_R://from kernel to user
			ret = copy_to_user((void*)args,&data,4);
			if(ret)
			{
				printk("copy data from kernel to user error!!!\n");
				return -EINVAL;
			}
			break;
		case ACCESS_DATA_W://from user to kernel
			ret = copy_from_user(&data,(void*)args,4);
			if(ret)
			{
				printk("copy data from user to kernel error!!!\n");
				return -EINVAL;
			}
			break;
		case ACCESS_STRING_R://from kernel to user (string type)
			ret = copy_to_user((void*)args,string_Buf,STRING_SIZE_R);
			if(ret)
			{
				printk("copy string from kernel to user error!!!\n");
				return -EINVAL;
			}
			break;
		case ACCESS_STRING_W://from user to kernel(string type)
			ret = copy_from_user(string_Buf,(void*)args,STRING_SIZE_W);
			if(ret)
			{
				printk("copy string from user to kernel error!!!\n");
				return -EINVAL;
			}
			printk("from user string data:%s\n",string_Buf);
			break;
		default:
			break;
	}

	return 0;
}


static struct file_operations fops = 
{
	.open			= myled_open,
	.read			= myled_read,
	.write			= myled_write,
	.release		= myled_close,
	.unlocked_ioctl = myled_ioctl,
};



//入口  进入驱动模块  __init 将入口函数放到__section(.init.text)
static int __init demo_init(void)
{
	printk("myled init\n");

	//注册字符驱动设备
	major = register_chrdev(major,CDEVNAME,&fops);
	if(major <= 0)
	{
		printk("register my cdev failed\n");
		return -EIO;//返回错误码
	}
	else
	{
		printk("register success\n");
		//物理地址与虚拟地址的映射
		if(myled_kernel_hw_init() == 0)//初始化成功
		{
			printk("virtual address OK!!\n");
		}
		else
		{
			return -ENOMEM;
		}
	}
	
	//4.创建设备节点
	cls = class_create(THIS_MODULE,"clq_class");
	if(IS_ERR(cls)){
		printk("class create error\n");
		return PTR_ERR(cls);
	}
	//MKDEV(ma,mi) //通过主设备和次设备号合成设备号
	//create three device to 3 led
	dev_red =  device_create(cls,NULL,MKDEV(major,MINOR_RED_LED),NULL,"myled_red");
	if(IS_ERR(dev_red)){
		printk("class device red led error\n");
		return PTR_ERR(dev_red);
	}

	dev_green =  device_create(cls,NULL,MKDEV(major,MINOR_GREEN_LED),NULL,"myled_green");
	if(IS_ERR(dev_green)){
		printk("class device green led error\n");
		return PTR_ERR(dev_green);
	}

	dev_blue =  device_create(cls,NULL,MKDEV(major,MINOR_BLUE_LED),NULL,"myled_blue");
	if(IS_ERR(dev_blue)){
		printk("class device blue led error\n");
		return PTR_ERR(dev_blue);
	}

	return 0;
}
//module_init:内核中的一个宏   将函数的名字告诉内核，以供内核回调
module_init(demo_init);


//出口  退出驱动模块 __exit 讲出口函数放到  __section(.exit.text)
static void __exit demo_exit(void)
{
	printk("myled exit\n");
	
	//1.注销设备节点
	device_destroy(cls,MKDEV(major,MINOR_RED_LED));
	device_destroy(cls,MKDEV(major,MINOR_GREEN_LED));
	device_destroy(cls,MKDEV(major,MINOR_BLUE_LED));
	class_destroy(cls);
	
	//取消地址映射
	iounmap(v_GPIOA);
	iounmap(v_GPIOB);
	iounmap(v_GPIOE);
	//注销字符驱动设备
	unregister_chrdev(major,CDEVNAME);
}
module_exit(demo_exit);


//许可证
MODULE_LICENSE("GPL");//遵从开源协议
//GNU

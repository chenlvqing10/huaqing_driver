#include "mycdev.h"

int mycdev_open(struct inode *__node, struct file *__fd)
{
	int which;
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);                      

	//提取设备号
	which = iminor(__node);
	printk("major:%d  minor:%d\n",imajor(__node),iminor(__node));

	//通过file结构体得到一些信息
	__fd->private_data = (void*)which;
	//printk("111:%d   222:%d\n",imajor(__fd->f_mapping->host),iminor(__fd->f_mapping->host));  
	return 0;                                                                                 
}                                                                                             

int mycdev_close(struct inode *__node, struct file *__fd)                                      
{                                                                                             
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);                                    
	return 0;                                                                                 
}   


ssize_t mycdev_read(struct file *file, char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf)) 
		size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret)
	{
		printk("copy data to user error");
		return -EIO ; //杩斿洖閿欒鐮�
	}

	return size;
}

ssize_t mycdev_write(struct file *file,const char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	int which;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf))
		size = sizeof(kbuf);
	ret = copy_from_user(kbuf,ubuf,size);
	if(ret)
	{
		printk("copy data from user error");
		return -EIO ; //杩斿洖閿欒鐮�
	}
	printk("kbuf = %s\n",kbuf);
	which = (int)file->private_data;

	switch(which)
	{
		case MINOR_RED_LED:
			(kbuf[0]=='1') ? (*(v_GPIOA + OUT) |=  (0x1 << 28)) : (*(v_GPIOA + OUT) &= ~(0x1 << 28));
			break;
		case MINOR_GREEN_LED:
			(kbuf[0]=='1') ? (*(v_GPIOE + OUT) |=  (0x1 << 13)) : ( *(v_GPIOE + OUT) &= ~(0x1 << 13));
			break;
		case MINOR_BLUE_LED:
			(kbuf[0]=='1') ? (*(v_GPIOB + OUT) |=  (0x1 << 12)) : (*(v_GPIOB + OUT) &= ~(0x1 << 12));
			break;
		default:
			break;
	}

	return size;
}

long mycdev_ioctl(struct file *file, unsigned int cmd, unsigned long args)                            
{                                                                                                    

	switch(cmd){                                                                                     
		default:                                                                        
			break;                                                                      
	}                                                                                   

	return 0;                                                                           
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

static struct file_operations fops =
{
	.open           = mycdev_open,
	.read           = mycdev_read,
	.write          = mycdev_write,
	.release        = mycdev_close,
	.unlocked_ioctl = mycdev_ioctl,
};


//入口 进入驱动模块
static int __init mycdev_init(void)
{
	int ret;
	int i=0;
	dev_t devno;//设备号

	//创建字符设备驱动的流程

	//1.定义结构体并分配内存
	cdev = cdev_alloc();
	if(cdev == NULL)//内存分配失败
	{
		printk("malloc memory for cdev error\n");
		ret = -ENOMEM;//OUT OF MEMORY
		goto ERR_STOP0;
	}

	//2.结构体的初始化   初始化cdev对象
	cdev_init(cdev,&fops);

	//3.申请设备号
	if(major > 0)//主设备号>0 静态分配设备号  否则动态分配设备号
	{
		ret = register_chrdev_region(MKDEV(major,minor),count,MYCDEVNAME);
		if(ret)
		{
			printk("static alloc device number error\n");
			ret = -EBUSY;
			goto ERR_STOP1;
		}
	}
	else
	{
		ret = alloc_chrdev_region(&devno,0,count,MYCDEVNAME);
		if(ret)
		{
			printk("dynamic alloc device number error\n");
			ret = -EINVAL;
			goto ERR_STOP1;
		}

		major = MAJOR(devno);//获得主设备号
		minor = MINOR(devno);//获得次设备号
	}

	//4.注册设备  创建设备号
	ret = cdev_add(cdev,MKDEV(major,minor),count);
	if(ret)
	{
		printk("register char device error\n");
		ret = -EAGAIN;
		goto ERR_STOP2;
	}

	//创建设备节点
	cls = class_create(THIS_MODULE,MYCDEVNAME);
	if(IS_ERR(cls))
	{
		printk("class create error\n");       
		ret = PTR_ERR(cls);
		goto ERR_STOP3;
	}
	for(i=0;i<count;i++)
	{
		dev = device_create(cls,NULL,MKDEV(major,i),NULL,"mycdev%d",i);
		if(IS_ERR(dev))                            
		{                                          
			printk("create device error\n");       
			ret = PTR_ERR(dev);
			goto ERR_STOP4;
		}                                          
	}

	//物理地址与虚拟地址的映射
	if(myled_kernel_hw_init() == 0)//初始化成功
	{
		printk("virtual address OK!!\n");
	}
	else
	{
		return -ENOMEM;
	}


	return 0;

ERR_STOP4:
	//注销设备节点
	for(--i;i>=0;i--)
	{
		device_destroy(cls,MKDEV(major,i));
	}
	class_destroy(cls);
ERR_STOP3:
	//字符设备驱动的注销
	cdev_del(cdev);
ERR_STOP2:
	//释放设备号
	unregister_chrdev_region(MKDEV(major,minor),count);
ERR_STOP1:
	//释放cdev对象
	kfree(cdev);
ERR_STOP0:
	return ret;
}

//出口  退出驱动模块
static void __exit mycdev_exit(void)
{
	int i = 3;
	//1.设备节点的注销
	for(--i;i>=0;i--)
	{
		device_destroy(cls,MKDEV(major,i));
	}
	class_destroy(cls);
	//2.字符设备驱动的注销
	cdev_del(cdev);
	//3.释放设备号
	unregister_chrdev_region(MKDEV(major,minor),count);
	//4.释放对象的内存
	kfree(cdev);
}

module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");

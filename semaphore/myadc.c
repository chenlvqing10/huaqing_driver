#include "myadc.h"

int myadc_open(struct inode *__node, struct file *__fd)
{
	if(down_trylock(&sem))
	{
		printk("try lock error by semaphore\n");
		return -EBUSY;
	}
	//上锁成功
	printk("semaphore lock success\n");

	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__); 

	return 0;                                                                                 
}                                                                                             

int myadc_close(struct inode *__node, struct file *__fd)                                      
{                                                                                             
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);        
	up(&sem);

	return 0;                                                                                 
}   


ssize_t myadc_read(struct file *file, char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	/*
	int digital_value = 0;//数字量
	int anolog_value  = 0;//模拟量

	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	//开启ADC转换  ADCCON[0]
	*(v_ADC_BASE + ADCCON) |= 0x1;

	//等待ADC转换结果  ADCCON[0]
	while((*(v_ADC_BASE + ADCCON) & 0x1));    //等待低电平  转换完成

	// 读取转换的数字量  ADCDAT[11:0]
	*(v_ADC_BASE + ADCDAT) &= 0xFFF;

	digital_value = *(v_ADC_BASE + ADCDAT);
	printk("ADCDAT =%d\n",digital_value);

	//将数字量转换为模拟量进行输出
	anolog_value = (int)(2 * digital_value * ((float)1800 / 4096));//1.8v  12位

	printk("转换后的ADC电压值 = %d\n",anolog_value);

	sprintf(kbuf,"%d",anolog_value);//整型数转字符串

*/
	if(size > sizeof(kbuf)) 
		size = sizeof(kbuf);
	ret = copy_to_user(ubuf,kbuf,size);
	if(ret)
	{
		printk("copy data to user error");
		return -EIO ; //input/output error
	}
	return size;
}

ssize_t myadc_write(struct file *file,const char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
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

	return size;
}

long myadc_ioctl(struct file *file, unsigned int cmd, unsigned long args)                            
{                                                                                                    

	switch(cmd){                                                                                     
		default:                                                                        
			break;                                                                      
	}                                                                                   

	return 0;                                                                           
}
int myadc_kernel_hw_init(void)//LED内核与硬件交互的初始化                      
{                                                                              
	int ret = 0;
	

	v_ADC_BASE  = ioremap(ADC_BASE,20);//20---5个寄存器地址                         
	v_IP_RESET1 = ioremap(IP_RESET1,4);//4----实际寄存器地址

	if(v_ADC_BASE == NULL)            
	{                                                                          
		ret = -ENOMEM;                                                         
	}                                                                          
	else                                                                       
	{     
		*(v_IP_RESET1)  &= (~(0x1 <<28));
		*(v_IP_RESET1)  |= (0x1 << 28);

		//设置ADCDAT读取数据的延时时间  ADCCON[13:10]
		*(v_ADC_BASE + ADCCON) &= ~(0xF << 10);//置0
		*(v_ADC_BASE + ADCCON) |= (ADC_DATA_SEL << 10);//延时周期为5个PCLK时钟周期

		//设置ADC上电延时多少时钟才能开启ADC转换 ADCCON[9:6]
		*(v_ADC_BASE + ADCCON) &= ~(0xF << 6);
		*(v_ADC_BASE + ADCCON)  |= (TOT_ADC_CLK_Cnt << 6);

		//ADC模拟输入通道的选择 ADCCON[5:3]
		*(v_ADC_BASE + ADCCON) &= ~(0x7 <<3);
		*(v_ADC_BASE + ADCCON) |= (ASEL << 3);

		// 设置ADC分频器的分频值   PRESCALERCON[9:0]
		*(v_ADC_BASE + PRESCALERCON) |= _PRESCALERCON;

		// 设置ADC分频器的使能位  PRESCALERCON[15]
		*(v_ADC_BASE + PRESCALERCON) |= (0x1 << 15);

		// 开启ADC的电源  ADCCON[2]
		*(v_ADC_BASE + ADCCON) &= ~(0x1 <<2);//0:power on

		//中断使能 ADCINTENB[0]=1
		*(v_ADC_BASE + ADCINTENB) = 0x0;
	}                                                                          

	return ret;                                                                
}                                                                              

static struct file_operations fops =
{
	.open           = myadc_open,
	.read           = myadc_read,
	.write          = myadc_write,
	.release        = myadc_close,
	.unlocked_ioctl = myadc_ioctl,
};


//入口 进入驱动模块
static int __init myadc_init(void)
{
	int ret;
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
	dev = device_create(cls,NULL,MKDEV(major,minor),NULL,"myadc");
	if(IS_ERR(dev))                            
	{                                          
		printk("create device error\n");       
		ret = PTR_ERR(dev);
		goto ERR_STOP4;
	}                                          

	//物理地址与虚拟地址的映射
	if(myadc_kernel_hw_init() == 0)//初始化成功
	{
		printk("virtual address OK!!\n");
	}
	else
	{
		return -ENOMEM;
	}

	//锁的初始化
	spin_lock_init(&lock);
	sema_init(&sem,1);
	mutex_init(&mutexlock);

	return 0;

ERR_STOP4:
	//注销设备节点
	device_destroy(cls,MKDEV(major,minor));
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
static void __exit myadc_exit(void)
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

module_init(myadc_init);
module_exit(myadc_exit);
MODULE_LICENSE("GPL");

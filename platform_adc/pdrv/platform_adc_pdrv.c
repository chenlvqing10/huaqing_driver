#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/clkdev.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

struct resource *res;//get resource object
unsigned int type[] = {IORESOURCE_MEM,IORESOURCE_IRQ};//get resource type
int irqno;

//driver name
#define MYDRIVERNAME "farsight_adc_irq"
//some define for cdev
struct cdev   *mycdev = NULL;	//cdev object
unsigned int  major   = 0;		//major device no
unsigned int  minor   = 0;		//minor device no
const int     count   = 1;		//same device count
struct class  *cls    = NULL;	//class dir object
struct device *dev    = NULL;	//drivers object

//register define		  
#define ADC_BASE 0xC0053000
#define ADCCON		 0 // 		 
#define ADCDAT		 1 //
#define ADCINTENB	 2 //
#define ADCINTCLR	 3 //
#define PRESCALERCON 4 //                   
static volatile  unsigned int* v_adc_base;//define virtual address   
//define for irq
#define ADC_IRQ_NO     73  //define adc irq
#define ADC_IRQ_NAME   "farsight_adc_irq"

//define file operation
char kbuf[128] = {0};//kernel buffer

//IO modle
struct semaphore sema;//define semaphore
wait_queue_head_t wq;//wait queue head
unsigned int condition = 0; //irq condition

/*init adc register set*/
static unsigned int adc_init(void)
{
	//virtual address ioremap
	int ret;
	v_adc_base = ioremap(ADC_BASE,20);
	if(v_adc_base == NULL)
	{
		printk("<0>" "virtual address map error\n");
		ret = -ENOMEM;
	}
	else
	{
		//1.clock enable
		//2.set power on delay timer  ADCCON[9:6]
		writel(readl(v_adc_base + ADCCON) & ~(0xF << 6),v_adc_base + ADCCON);
		writel(readl(v_adc_base + ADCCON) | (0x6  << 6),v_adc_base + ADCCON);

		//3.set read adc date delay timer  ADCCON[13:10]
		writel(readl(v_adc_base + ADCCON) & ~(0xF << 10),v_adc_base + ADCCON);

		//4.set prescalercon data  PRESCALERCON[9:0]=199  PRESCALERCON[15] = 1
		writel(readl(v_adc_base + PRESCALERCON) | (0x1 << 15),v_adc_base + PRESCALERCON);
		writel(readl(v_adc_base + PRESCALERCON) | (199 << 0), v_adc_base + PRESCALERCON);

		//5.set adc channel 0  ADCCON[5:3]
		writel(readl(v_adc_base + ADCCON) & ~(0x7 << 3),v_adc_base + ADCCON);

		//6.start adc power  ADCCON[2] 0:power on
		writel(readl(v_adc_base + ADCCON) & ~(0x1 << 2),v_adc_base + ADCCON);
		
		//7.enable intterupt ADCINTENB[0]=1
		writel(1,v_adc_base + ADCINTENB);

		//8.enable read intterupt pending ADCINTCLR[0]=1
		writel(1,v_adc_base + ADCINTCLR);

	}
	return 0;
}static int Farsight_adc_open(struct inode *node, struct file *file)
{
	printk("<0>" "%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}



//read data to user space
ssize_t Farsight_adc_read(struct file *file, char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	int dgt_voltage = 0;//digital voltage
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);

	if(size > sizeof(kbuf))
		size = sizeof(kbuf);

	//lock semaphore
	if(down_trylock(&sema))
	{
		printk("<0>" "get lock error\n");
		return -EBUSY;
	}
	else//lock success get adc resource
	{
		//start adc change  ADCCON[0]
		writel((readl(v_adc_base) |(1<<0)),v_adc_base);

		if(file->f_flags & O_NONBLOCK)
		{
			return -EINVAL;
		}
		else
		{
			ret = wait_event_interruptible(wq,condition);
			if(ret){
				printk("wait error\n");
				return ret;
			}
		}
		//after waking up then read ADC  ADCDAT[11:0]
		dgt_voltage = readl(v_adc_base + ADCDAT);
		dgt_voltage &= 0xFFF;
		//put voltage copy to user
		ret = copy_to_user(ubuf,&dgt_voltage,size);//interger data
		if(ret)
		{
			printk("<0>" "copy kernel data to user error");
			return -EIO;
		}

	}
	condition = 0;
	up(&sema);//unlock
	return size;
}

static int Farsight_adc_close(struct inode *node, struct file *file)
{
	printk("%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

static struct file_operations fops = {
	.open    = Farsight_adc_open,
	.read    = Farsight_adc_read,
	.release = Farsight_adc_close,
};
//irq handler function
static irqreturn_t handler_farsight_irq_adc(int irqno,void* arg)
{
	//wake up wait queue
	condition = 1;
	wake_up_interruptible(&wq);//wake up lock
	//清除adc的中断状态标志位
	*(v_adc_base + ADCINTCLR) = 0x1;//irq del  	

	return IRQ_HANDLED;
}

//create platform_driver struct and init
int pdrv_probe(struct platform_device *pdev)
{
	int ret;
	dev_t devno;
	printk("%s::%s::%d\n",__FILE__,__func__,__LINE__);

	//get device resource
	res = platform_get_resource(pdev,type[0],0);
	if(res == NULL){
		printk("platform get resource[0] error\n");
		return -EAGAIN;
	}
	//get irq resource
	irqno = platform_get_irq(pdev,0);
	if(irqno < 0)
	{
		printk("platform get irqno error\n");
		return irqno;//return errorno
	}
	printk("<0>" "addr = %#x,irqno = %d\n",res->start,irqno);


	/*create char driver*/
	//1.define the cdev struct and malloc memory
	mycdev = cdev_alloc();
	if(mycdev == NULL)
	{
		printk("<0>" "alloc memory for cdev error\n");
		ret = -ENOMEM;
		goto ERR_STOP0;
	}
	//2.init cdev struct
	cdev_init(mycdev,&fops);
	//3.register device no
	if(major > 0)//static alloc device no(alloc by programmer)
	{
		ret = register_chrdev_region(MKDEV(major,minor),count,MYDRIVERNAME);
		if(ret)
		{
			printk("<0>" "static alloc device no error\n");
			ret = -EBUSY;
			goto ERR_STOP1;
		}
	}
	else//dynamic alloc device no(alloc by kernel)
	{
		ret = alloc_chrdev_region(&devno,0,count,MYDRIVERNAME);
		if(ret)
		{
			printk("<0>" "dynamic alloc device no error\n");
			ret = -ENAVAIL;
			goto ERR_STOP1;
		}
		//get the major and minor
		major = MAJOR(devno);
		minor = MINOR(devno);
		printk("<0>" "major = %d  minor = %d \n",major,minor);
	}
	//4.add char device by the getting devno  
	//exec 1 2 3 4 then successed create a cdev object
	ret = cdev_add(mycdev,MKDEV(major,minor),count);
	if(ret)
	{
		printk("<0>" "create device  error\n");
		ret = -EAGAIN;
		goto ERR_STOP2;
	}
	//ADC init
	if((ret=adc_init()))
	{
		printk("<0>" "init adc register error\n");
		return ret;
	}
	else
	{
		printk("<0>" "adc init OK\n");
	}


	//irq init
	ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_NONE,ADC_IRQ_NAME,NULL);//
	if(ret)
	{
		printk("<0>" "register irq_adc : 73 error\n");
		goto ERR_STOP0;
	}
	//init wait queue
	 init_waitqueue_head(&wq);

	//semqphore init
	sema_init(&sema,1);
	//5.crreate device node
	cls = class_create(THIS_MODULE,MYDRIVERNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR_STOP3;
	}

	dev = device_create(cls,NULL,MKDEV(major,minor),NULL,MYDRIVERNAME);
	if(IS_ERR(dev))
	{
		printk("device create myadc error\n");
		ret = PTR_ERR(dev);
		goto ERR_STOP4;
	}   

	return 0;

ERR_STOP4:
	device_destroy(cls,MKDEV(major,minor));
	class_destroy(cls);
ERR_STOP3:
	cdev_del(mycdev);//del char device
ERR_STOP2:
	unregister_chrdev_region(MKDEV(major,minor),count);//free regisger chrdev
ERR_STOP1:
	kfree(mycdev);//free cdev object
ERR_STOP0:
	return ret;
}

int pdrv_remove(struct platform_device *pdev)
{
	printk("%s::%s::%d\n",__FILE__,__func__,__LINE__);
	//destroy device node
	device_destroy(cls,MKDEV(major,minor));
	class_destroy(cls);

	//del char device
	cdev_del(mycdev);

	//free regisger chrdev
	unregister_chrdev_region(MKDEV(major,minor),count);

	//free cdev object
	kfree(mycdev);

	//free ioremap
	iounmap(v_adc_base);

	//ADC IRQ DELETE AND POWER OFF
	*(v_adc_base + ADCCON) |= (0x1 << 2);//1:power off
	*(v_adc_base + ADCINTCLR) = 0x1;//irq del

	//free irq
	free_irq(ADC_IRQ_NO,NULL);
	return 0;
}

struct platform_device_id pdrv_idtable[] = 
{
	{"platform_adc",},
	{"platform_adc01",},
	{"platform_adc02",},
	{"platform_adc03",},
	{"platform_adc04",},
	{},//end and exit while
};

struct platform_driver pdrv =
{
	.probe   = pdrv_probe,//device and driver match success then exec this fun
	.remove  = pdrv_remove,//move device and driver then exec this fun
	.driver  = {
		.name = "platform_adc",
	},
	.id_table = pdrv_idtable,
};


module_platform_driver(pdrv);//enter register unregister exit fun
MODULE_LICENSE("GPL");





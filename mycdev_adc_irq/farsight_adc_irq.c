#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/string.h>
#include "cmd.h"
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

//driver name
#define MYDRIVERNAME "farsight_adc_irq"
//some define for cdev
struct cdev   *mycdev = NULL;	//cdev object
unsigned int  major   = 0;		//major device no
unsigned int  minor   = 0;		//minor device no
const int     count   = 1;		//same device count
struct class  *cls    = NULL;	//class dir object
struct device *dev    = NULL;	//drivers object
struct adc_dev *adc_dev;

//some defint for ADC
struct ADC_DEV
{
	unsigned int ASEL;//adc input channel
	unsigned int ADC_DATA_SEL;
	unsigned int TOT_ADC_CLK_Cnt;
	unsigned int prescalercon;
};

struct ADC_DEV* myadc_dev;

#define ADC_BASE 0xC0053000
//register define		  
#define ADCCON		 0 // 		 
#define ADCDAT		 1 //
#define ADCINTENB	 2 //
#define ADCINTCLR	 3 //
#define PRESCALERCON 4 //                   
static volatile  unsigned int* v_adc_base;//define virtual address   
int  ang_voltage = 0;//analogy voltage


//define for irq
#define ADC_IRQ_NO     73  //define adc irq
#define ADC_IRQ_NAME   "farsight_adc_irq"


//define file operation
char kbuf[128] = {0};//kernel buffer



//IO modle
struct semaphore sema;//define semaphore
wait_queue_head_t wq;//wait queue head
DECLARE_WAIT_QUEUE_HEAD(wq);//init wait head queue
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
		myadc_dev = kzalloc(sizeof(struct ADC_DEV),GFP_KERNEL);//alloc mem or segament error
		//1.clock enable
		//2.set power on delay timer  ADCCON[9:6]
		myadc_dev->TOT_ADC_CLK_Cnt = 6;//6 clock timer
		*(v_adc_base + ADCCON) &= ~(0xF << 6);
		*(v_adc_base + ADCCON) |= ~(myadc_dev->TOT_ADC_CLK_Cnt << 6);

		//3.set read adc date delay timer  ADCCON[13:10]
		myadc_dev->ADC_DATA_SEL = 0;//5 clock timer
		*(v_adc_base + ADCCON) &= ~(0xF << 10);
		*(v_adc_base + ADCCON) |= ~(myadc_dev->ADC_DATA_SEL << 10);

		//4.set prescalercon data  PRESCALERCON[9:0]
		myadc_dev->prescalercon = 199;
		*(v_adc_base + PRESCALERCON) |= (0x1 << 15);//enable prescalercon  RESCALERCON[15]
		*(v_adc_base + PRESCALERCON) |= myadc_dev->prescalercon;

		//5.set adc channel 0  ADCCON[5:3]
		myadc_dev->ASEL = 0;
		*(v_adc_base + ADCCON) &= ~(0x7 << 3);
		*(v_adc_base + ADCCON) |= (myadc_dev->ADC_DATA_SEL << 3);

		//6.start adc power  ADCCON[2]
		*(v_adc_base + ADCCON) &= ~(0x1 << 2);//0:power on

		//7.enable intterupt ADCINTENB[0]=1
		*(v_adc_base + ADCINTENB) = 0x1;

	}
	return 0;
}


//printk("<0>" );



static int Farsight_adc_open(struct inode *node, struct file *file)
{
	printk("<0>" "%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}



//read data to user space
ssize_t Farsight_adc_read(struct file *file, char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	printk("<0>" "%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);

	if(size > sizeof(kbuf))
		size = sizeof(kbuf);

	//lock semaphore
	if(down_trylock(&sema))
	{
		printk("<0>" "get lock error\n");
		return -EBUSY;
	}
	else
	{
		//lock success get adc resource
		//start adc change  ADCCON[0]
		*(v_adc_base + ADCCON) = 0X01;
		wait_event_interruptible(wq,condition);//wait irq wake

		//put voltage copy to user
		sprintf(kbuf,"%d",ang_voltage);//int to string
		printk("<0>" "kbuf=%s\n",kbuf);
		ret = copy_to_user(ubuf,kbuf,sizeof(kbuf));
		if(ret)
		{
			printk("<0>" "copy kernel data to user error");
			return -EIO;
		}
		up(&sema);//unlock

	}

	return size;
}



//write data to kernel
ssize_t Farsight_adc_write(struct file *file, char __user *ubuf,size_t size, loff_t *offs)
{
	int ret;
	printk("<0>" "%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	if(size > sizeof(kbuf))
		size = sizeof(kbuf);

	ret = copy_from_user(kbuf,ubuf,size);
	if(ret)
	{
		printk("<0>" "copy data to kernel from user error");
		return -EIO;
	}

	printk("<0>" "write kbuf = %s\n",kbuf);

	return size;
}
static int Farsight_adc_close(struct inode *node, struct file *file)
{
	printk("<0>" "%s::%s::%d\n",__FILE__,__FUNCTION__,__LINE__);
	return 0;
}

static struct file_operations fops = {
	.open    = Farsight_adc_open,
	.read    = Farsight_adc_read,
//	.write   = Farsight_adc_write,
	.release = Farsight_adc_close,
};


//irq handler function
static irqreturn_t handler_farsight_irq_adc(int irqno,void* arg)
{
	int dgt_voltage = 0;//digital voltage


	//irq start ADC  ADCDAT[11:0]
	*(v_adc_base+ ADCDAT) &= 0xFFF;

	dgt_voltage =  *(v_adc_base+ ADCDAT);
	ang_voltage = (int)( dgt_voltage * 2 * ( (float) 1800 / 4096));
	printk("dgt_voltage  = %d  ang_voltage =%d \n",dgt_voltage,ang_voltage);
	condition = 1;
	wake_up_interruptible(&wq);//wake up lock

	return IRQ_HANDLED;
}

//device Enter
int __init farsight_irq_init(void)
{
	int ret;
	int i;
	dev_t  devno;//device no

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
		printk("<0>" "major = %d\n  minor = %d \n",major,minor);
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
	//5.crreate device node
	cls = class_create(THIS_MODULE,MYDRIVERNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR_STOP3;
	}

	for(i=0;i<count;i++){
		dev = device_create(cls,NULL,
				MKDEV(major,i),NULL,MYDRIVERNAME"%d",i);
		if(IS_ERR(dev)){
			printk("device create myadc%d error\n",i);
			ret = PTR_ERR(dev);
			goto ERR_STOP4;
		}
		
	}   

	//ADC init
	if((ret=adc_init()))
	{
		printk("<0>" "init adc register error\n");
		return ret;
	}

	//irq init
	ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_NONE,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_SHARED,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_FALLING,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_RISING,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_HIGH,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_LOW,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_MASK,ADC_IRQ_NAME,NULL);//
	//ret = request_irq(ADC_IRQ_NO,handler_farsight_irq_adc,IRQF_TRIGGER_PROBE,ADC_IRQ_NAME,NULL);//
	if(ret)
	{
		printk("<0>" "register irq_adc : 73 error\n");
		goto ERR_STOP0;
	}

	//semqphore init
	sema_init(&sema,1);
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


//device exit
static void __exit farsight_irq_exit(void)
{
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

	//destroy clock


}


module_init(farsight_irq_init);
module_exit(farsight_irq_exit);
MODULE_LICENSE("GPL");


#include <linux/init.h>      
#include <linux/kernel.h>
#include <linux/module.h>

//内核模块参数
static int  baudrate = 9600;
static int  port[4]  = {0,1,2,3};
static char c        = 'a'; 
static char *name	 = "demo";

module_param(baudrate,int,S_IRUGO);
module_param_array(port,int,NULL,S_IRUGO);
module_param(c,byte,S_IRUGO);
module_param(name,charp,S_IRUGO);

MODULE_PARM_DESC(baudrate,"the serial transmission baudrate");

//入口  进入驱动模块  __init 将入口函数放到__section(.init.text)
static int __init demo_init(void)
{
	int i;
	printk("demo init\n");
	printk("file:%s  function:%s  line:%d\n",__FILE__,__FUNCTION__,__LINE__);
	
	//打印参数
	printk("baudrate = %d\n",baudrate);
	printk("port:");
	for(i=0;i<ARRAY_SIZE(port);i++)
		printk("port[%d] = %d  \n",i,port[i]);
	printk("\n");
	printk("c=%c\n",c);
	printk("name=%s\n",name);


	return 0;
}
//module_init:内核中的一个宏   将函数的名字告诉内核，以供内核回调
module_init(demo_init);


//出口  退出驱动模块 __exit 讲出口函数放到  __section(.exit.text)
static void __exit demo_exit(void)
{
	printk("demo exit\n");
}
module_exit(demo_exit);


//许可证
MODULE_LICENSE("GPL");//遵从开源协议
//GNU

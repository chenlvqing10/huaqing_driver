#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

struct resource *res[2];//get resource object
unsigned int type[] = {IORESOURCE_MEM,IORESOURCE_IRQ};//get resource type
int irqno;

//create platform_driver struct and init
int pdrv_probe(struct platform_device *pdev)
{
	int i;
	printk("%s::%s::%d\n",__FILE__,__func__,__LINE__);

	//get device resource
	for(i=0;i<ARRAY_SIZE(res);i++){
		res[i] = platform_get_resource(pdev,type[i],0);
		if(res[i] == NULL){
			printk("platform get resource[%d] error\n",i);
			return -EAGAIN;
		}
	}
	printk("<0>"  "addr = %#x,irqno = %d\n",res[0]->start,res[1]->start);
	
	//get irq resource
	irqno = platform_get_irq(pdev,0);
	if(irqno < 0)
	{
		printk("platform get irqno error\n");
		return irqno;//return errorno
	}
	printk("<0>" "irqno = %d\n",irqno);
	
	return 0;
}

int pdrv_remove(struct platform_device *pdev)
{
	printk("%s::%s::%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

/*
struct platform_device_id pdrv_idtable[] = 
{
		{"hello_adc",},
		{"hello_adc01",},
		{"hello_adc02",},
		{"hello_adc03",},
		{"hello_adc04",},
		{},//end and exit while
};
*/

struct platform_driver pdrv =
{
	.probe   = pdrv_probe,//device and driver match success then exec this fun
	.remove  = pdrv_remove,//move device and driver then exec this fun
	.driver  = {
				.name = "hello_adc",
	},
//	.id_table = pdrv_idtable,
};


/*
//driver enter
static int __init pdrv_init(void)
{
	return platform_driver_register(&pdrv);//register driver
}

//driver exit
static void __exit pdrv_exit(void)
{
	platform_driver_unregister(&pdrv);//unregister driver
}


module_init(pdrv_init);
module_exit(pdrv_exit);
*/
module_platform_driver(pdrv);
MODULE_LICENSE("GPL");





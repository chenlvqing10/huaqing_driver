#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

//create platform_device struct and init
struct resource res[] = 
{
	[0] = {
			.start = 0xC0053000,
			.end   = 0xC0053000 + 24 -1,
			.flags = IORESOURCE_MEM,//Memory resource
	},

	[1] = {
			.start = 73,
			.end   = 73,
			.flags = IORESOURCE_IRQ,//irq resource
	},	
};

void pdev_release(struct device *dev)
{
	printk("%s::%s::%d\n",__FILE__,__func__,__LINE__);
}


struct platform_device pdev =
{
	.name = "platform_adc",
	.id   = -1,
	.dev  ={
			.release = pdev_release,
		},
	.num_resources = ARRAY_SIZE(res),
	.resource     = res,
};



//driver enter
static int __init pdev_init(void)
{
	return platform_device_register(&pdev);//register device
}

//driver exit
static void __exit pdev_exit(void)
{
	platform_device_unregister(&pdev);//unregister device
}


module_init(pdev_init);
module_exit(pdev_exit);
MODULE_LICENSE("GPL");





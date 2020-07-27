#include <linux/init.h>      
#include <linux/kernel.h>
#include <linux/module.h>

extern  int  expval;//调用A模块的变量
extern  void expfun(void);//调用A模块的函数

//入口  进入驱动模块  __init 将入口函数放到__section(.init.text)
static int __init demo_init(void)
{
	printk("demoB init\n");

	//使用A模块的变量和函数
	printk("expval =%d\n",expval);
	expfun();

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

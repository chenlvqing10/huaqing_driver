#include <linux/init.h>                                                                                                                       
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/timer.h>


#define GPIONO(m,n)   (32*m+n)
#define GPIOB8        GPIONO(1,8)      //left key
#define GPIOB16       GPIONO(1,16)    //right key
#define GPIOA28       GPIONO(0,28)    //red led
#define GPIOB12       GPIONO(1,12)   //green led
#define GPIOE13       GPIONO(4,13)   //blue led


//定时器对象
struct timer_list mytimer;

//key irq
unsigned int irqno[2] = {GPIOB8,GPIOB16};
char* irqname[2] = {"interrupt_gpio8_leftkey","interrupt_gpio16_rightkey"};

//led gpiono
unsigned int gpio_io[3] = {GPIOA28,GPIOB12,GPIOE13};

//中断处理函数
irqreturn_t handler_farsight_irq(int irqno,void* arg)
{
    //重新启动定时器
    mod_timer(&mytimer,jiffies + 10);
     return IRQ_HANDLED;
 }
 
 
 //定时器处理函数
 void   irq_timer_function(unsigned long data)
 {
     unsigned int gpiob8_status = gpio_get_value(GPIOB8);
     unsigned int gpiob16_status = gpio_get_value(GPIOB16);
     unsigned int red,green;
     
     //判断管脚是否是低电平
     //在管脚的任何模式下都可以读
     if(gpiob8_status == 0)
     {
         //红灯取反
         red = gpio_get_value(GPIOA28);
         red = !red;
		 
		 gpio_set_value(GPIOE13,0);//set green led to low
         gpio_set_value(GPIOA28,red);//low-->high-->low-->...
         
         printk("<0>" "left key down+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
     }
 
     if(gpiob16_status == 0)
     {
         //绿灯取反
         green = gpio_get_value(GPIOE13);
         green = !green;

		 gpio_set_value(GPIOA28,0);//set red led to low
         gpio_set_value(GPIOE13,green);
         printk("<0>" "right key down-----------------------------------------------------\n");
     }
 }

//内核驱动入口
int __init farsight_irq_init(void)
{
    int ret;
    int i;

    //定时器初始化
    mytimer.expires    = jiffies + 10;
    mytimer.function   = irq_timer_function;
    mytimer.data       = 0;
    init_timer(&mytimer);
    //启动定时器
    add_timer(&mytimer);
    
    //初始化RGB led灯
    for(i=0;i<ARRAY_SIZE(gpio_io);i++)
    {
        gpio_free(gpio_io[i]);
    }
        
    for(i=0;i<ARRAY_SIZE(gpio_io);i++)
    {
        ret = gpio_request(gpio_io[i],NULL);//申请gpio

        if(ret)
        {
                printk("<0>" "request gpio %d error\n",gpio_io[i]);
                return ret;
        }
    
        gpio_direction_output(gpio_io[i],0);//设置成输出模式 低电平
    }
    
    //初始化中断
    for(i=0;i<ARRAY_SIZE(irqno);i++)
    {
        ret = request_irq(gpio_to_irq(irqno[i]),handler_farsight_irq,IRQF_TRIGGER_FALLING,irqname[i],NULL);
        if(ret)
        {                                                                                                                        
            printk("<0>" "register irq %s : %d error\n",irqname[i],gpio_to_irq(irqno[i]));
            return ret;
        }
    }


    return 0;
}



//内核驱动退出
static void __exit farsight_irq_eixt(void)
{
    int i;
    //注销中断
    for(i=0;i<ARRAY_SIZE(irqno);i++)
    {
        free_irq(gpio_to_irq(irqno[i]),NULL);
    }
	printk("<0>" "key interrupt free\n");

    //注销定时器
    del_timer(&mytimer);
	printk("<0>" "timer free\n");

    //注销gpio子系统 for RGB led
    for(i=0;i<ARRAY_SIZE(gpio_io);i++)
    {
        gpio_free(gpio_io[i]);
    }
	printk("<0>" "gpio free for led\n");
}

module_init(farsight_irq_init);
module_exit(farsight_irq_eixt);
MODULE_LICENSE("GPL");



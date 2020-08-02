#ifndef  __MYCDEV__
#define  __MYCDEV__

//head file
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
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/spinlock_types.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/wait.h>

//自旋锁机制
spinlock_t lock;
//信号量机制
struct semaphore sem;
//互斥体
struct mutex mutexlock;
//原子操作
atomic_t atm = ATOMIC_INIT(-1);
//
int flags = 0;

//IO模型
//阻塞模型
int condition = 0; //休眠条件  0--休眠状态
wait_queue_head_t wq; //定义等待队列头
DECLARE_WAIT_QUEUE_HEAD(wq);//初始化队列头

#define  MYCDEVNAME  "myadc"
typedef unsigned int  _uint32_t;   
typedef unsigned long _uint64_t;   
typedef unsigned char _uint8_t;    

//字符设备驱动---adc
struct cdev  *cdev = NULL;
unsigned int major = 0;
unsigned int minor = 0;
const    int count = 1;
struct class *cls  = NULL;//目录名         
struct device *dev = NULL;//驱动名
char kbuf[128] = "kernal strings";

//设置寄存器的基地址           
#define ADC_BASE 0xC0053000    
//寄存器地址偏移量            
#define ADCCON       0 //控制寄存器          
#define ADCDAT		 1 //输出数据寄存器  获取数字量
#define ADCINTENB    2 //中断使能寄存器
#define ADCINTCLR    3 //中断清除寄存器
#define PRESCALERCON 4 //预分频器
#define  ADC_DATA_SEL       0       //DCDAT读取数据的延时时间 0000 延时5个周期
#define  TOT_ADC_CLK_Cnt    6       //设置ADC上电延时多少时钟才能开启ADC转换  至少6个周期之后才开始转换
#define  ASEL               0       //ADC模拟输入通道0
#define  _PRESCALERCON      199     //分频数值 >=25  25---254

//复位ADC控制器
#define IP_RESET1 0xC0012004

//定义虚拟地址                         
static volatile  _uint32_t* v_ADC_BASE;//虚拟基地址 
static volatile  _uint32_t* v_IP_RESET1;//复位ADC控制器虚拟地址

#endif

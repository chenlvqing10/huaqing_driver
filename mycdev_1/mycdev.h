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


#define  MYCDEVNAME  "mycdev"
typedef unsigned int  _uint32_t;   
typedef unsigned long _uint64_t;   
typedef unsigned char _uint8_t;    

//define cdev
struct cdev  *cdev = NULL;
unsigned int major = 600;
unsigned int minor = 0;
const    int count = 3;
struct class *cls  = NULL;//目录名         
struct device *dev = NULL;//驱动名        
//为led设备分配次设号         
#define MINOR_RED_LED    0    
#define MINOR_GREEN_LED  1    
#define MINOR_BLUE_LED   2    

char kbuf[128] = "kernal strings";

//设置寄存器的基地址           
#define GPIOA    0xC001A000    
#define GPIOE    0xC001E000    
#define GPIOB    0xC001B000    
//寄存器地址偏移量            
#define OUT       0           
#define OUTENB    1           
#define ALTFN0    8           
#define ALTFN1    9           

//定义虚拟地址                         
static volatile  _uint32_t* v_GPIOA;   
static volatile  _uint32_t* v_GPIOE;   
static volatile  _uint32_t* v_GPIOB;   




#endif

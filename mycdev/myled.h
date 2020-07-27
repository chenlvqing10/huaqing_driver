#ifndef __MYLED__
#define __MYLED__
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

//为led设备分配次设号
#define MINOR_RED_LED    0
#define MINOR_GREEN_LED  1
#define MINOR_BLUE_LED   2

typedef unsigned int  _uint32_t;
typedef unsigned long _uint64_t;
typedef unsigned char _uint8_t;

#define CDEVNAME "myled"    //驱动名
int major = 0;              //主设备号
int minor = 0;              //次设备号
char kbuf[128]={0};         //内核空间

struct class *cls = NULL;//目录名
struct device *dev_red = NULL;//红色LED灯设备节点
struct device *dev_green = NULL;//绿色LED灯设备节点
struct device *dev_blue = NULL;//蓝色LED灯设备节点

//命令设置
#define RED    '0'
#define GREEN  '1'
#define BLUE   '2'
#define STRING_SIZE_R ((ACCESS_STRING_R >> 16)&0x3FFF)
#define STRING_SIZE_W ((ACCESS_STRING_W >> 16)&0x3FFF)
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

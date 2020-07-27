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

typedef unsigned int  _uint32_t;
typedef unsigned long _uint64_t;
typedef unsigned char _uint8_t;

#define CDEVNAME "myled"    //驱动名
int major = 0;              //主设备号
char kbuf[128]={0};         //内核空间

//命令设置
#define RED    '0'
#define GREEN  '1'
#define BLUE   '2'

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

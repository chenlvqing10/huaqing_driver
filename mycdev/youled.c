#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define CDEVNAME "myled"
#define RED   48
#define GREEN 49
#define BLUE  50

unsigned int *red_base   = NULL;
unsigned int *green_base = NULL;
unsigned int *blue_base  = NULL;
int major = 0;
char kbuf[128] = "";
int myled_open(struct inode *inode, struct file *file)
{
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	return 0;
}
ssize_t myled_read(struct file *file, char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	if(size > sizeof(kbuf))
		size = sizeof(kbuf);
	ret = copy_to_user(ubuf, kbuf, size);
	if (ret) {
		printk("copy data to user error\n");
		return -EIO;
	}
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	return size;

}
ssize_t myled_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offs)
{
	int ret;
	if (size > sizeof(kbuf))
		size = sizeof(kbuf);
	ret = copy_from_user(kbuf, ubuf, size);
	if (ret) {
		printk("copy data from user error\n");
		return -EIO;
	}
	printk("kbuf = %s\n", kbuf);
	printk("kbuf[0]=%c\n",kbuf[0]);
	printk("kbuf[0]=%d\n",kbuf[0]);
	printk("kbuf[1]=%c\n",kbuf[1]);
	printk("kbuf[1]=%d\n",kbuf[1]);
	switch (kbuf[0]) {
	case RED:
		if (kbuf[1] == '1') {
			printk("let red ondddd\n");
			*red_base |= (0x1 << 28); 
			//红灯亮
		} else if (kbuf[1] == '0') {
			printk("let red offdddd\n");
			*red_base &= (~(0x1 << 28));
			//红灯灭
		}
		break;	
	case GREEN:
		if (kbuf[1] == '1') {
			printk("let green ondddd\n");
			*green_base |= (0x1 << 13);
			//绿灯亮
		} else if (kbuf[1] == '0') {
			printk("let green offdddd\n");
			*green_base &= (~(0x1 << 13));
			//绿灯灭
		}
		break;
	case BLUE:
		if (kbuf[1] == '1') {
			printk("let blue ondddd\n");
			*blue_base |= (0x1 << 12);
			//蓝灯亮
		} else if (kbuf[1] == '0') {
			printk("let blue offdddd\n");
			*blue_base &= (~(0x1 << 12));
			//蓝灯灭
		}
		break;
	}
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	return 0;

}

int myled_close(struct inode *inode, struct file *file)
{
	printk("%s %s %d\n", __FILE__, __func__, __LINE__);
	return 0;

}
static struct file_operations fops = {
	.open = myled_open,
	.read = myled_read,
	.write = myled_write,
	.release = myled_close,
};
//入口
static int __init demo_init(void)
{
	//注册字符设备驱动
	major = register_chrdev(major, CDEVNAME, &fops);
	if(major < 0) {
		printk("register char devices driver error\n");
		return major;
	}
	//地址映射
	red_base = ioremap(0xc001a000, 40);
	if (red_base == NULL) {
		printk("red ioremap error\n");
		return -ENOMEM;
	}
	green_base = ioremap(0xc001e000, 40);
	if (green_base == NULL) {
		printk("green ioremap error\n");
		return -ENOMEM;
	}
	blue_base = ioremap(0xc001b000, 40);
	if (blue_base == NULL) {
		printk("blue ioremap error\n");
		return -ENOMEM;
	}

	//所有灯都熄灭
	*red_base       &= (~(0x1 << 28)); // 红灯熄灭
	*(red_base + 1) |= (0x1 << 28); //设置为输出模式
	*(red_base + 9) &= ~(0x3 << 24); //设置GPIO功能

	*green_base       &= (~(0x1 << 13)); //绿灯熄灭
	*(green_base + 1) |= (0x1 << 13); //设置为输出模式
	*(green_base + 8) &= ~(0x3 << 26); //设置为GPIO功能

	*blue_base       &= (~(0x1 << 12)); //蓝灯熄灭
	*(blue_base + 1) |= (0x1 << 12); //设置为输出模式
	*(blue_base + 8) &= (~(0x3 << 24)); //设置GPIO功能
	*(blue_base + 8) |= (0x2 << 24);

	printk("ininininitijit\n");

	return 0;
}

//出口
static void __exit demo_exit(void)
{
	//取消映射
	iounmap(red_base);
	iounmap(blue_base);
	iounmap(green_base);

	//注销字符设备驱动
	unregister_chrdev(major, CDEVNAME);
	return ;
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");


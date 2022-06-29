/*
 * File Name: Hello.c
 *
 * Descriptions: 
 *		This is the first module.
 *
 * Author: 
 *		Zheng Qiang
 * Kernel Version: 2.6.29
 *
 * Update:
 * 		-	2009.12.19	Zheng Qiang	 Creat this file
 */                                                   
#include <linux/init.h>				/* ������һЩ��صĺ� */
#include <linux/module.h>			/* ������ģ����Ҫ��*/

static int hello_init(void)
{
	printk(KERN_ALERT "Hello, world\n");	/* ��ӡhello World */
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, world\n");	/* ��ӡGoodbye,world */
}

module_init(hello_init);   /* ָ��ģ����غ��� */
module_exit(hello_exit);   /* ָ��ģ��ж�غ��� */
MODULE_LICENSE("Dual BSD/GPL");
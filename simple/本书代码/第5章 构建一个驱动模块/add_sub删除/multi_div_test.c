/*
 * File Name: multi_div_test.c
 *
 * Descriptions: 
 *		Test the multi and div.
 *
 * Author: 
 *		Zheng Qiang
 * Kernel Version: 2.6.29
 *
 * Update:
 * 		-	2009.12.20	Zheng Qiang	 Creat this file
 */                                                   
#include <linux/init.h>				/* ������һЩ��صĺ� */
#include <linux/module.h>			/* ������ģ����Ҫ��*/
#include <multi_div.h>

static long a = 1;  
static long b = 1;  
  
static int init_test(void)
{
	long c=0;
	c=multiply_test(a,b);
	printk(KERN_ALERT "c=%ld*%ld=%ld",a,b,c);	/*  */
	c=divide_test(a,b);
	printk(KERN_ALERT "c=%ld/%ld=%ld",a,b,c);
	return 0;
}

static void exit_test(void)
{
	printk(KERN_ALERT "exit modules");	/* ��ӡGoodbye,world */
}

module_init(init_test);   /* ָ��ģ����غ��� */
module_exit(exit_test);   /* ָ��ģ��ж�غ��� */
module_param(a, long, S_IRUGO);  
module_param(b, long, S_IRUGO);  
/* ������Ϣ */                               
MODULE_LICENSE("Dual BSD/GPL");      
MODULE_AUTHOR("Zheng Qiang");
MODULE_DESCRIPTION("A Module for testing module params and EXPORT_SYMBOL");
MODULE_VERSION("V1.0");

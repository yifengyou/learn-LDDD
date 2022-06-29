
#include <linux/config.h>//����ͷ�ļ�
#include <linux/kernel.h>//�ں�ͷ�ļ�
#include <linux/sched.h>
#include <linux/timer.h>//ʱ��ͷ�ļ�
#include <linux/init.h>//�û�����ģ���ʼ�����������õ�ͷ�ļ�
#include <linux/module.h>//ģ����ص�ͷ�ļ�
#include <asm/hardware.h>

// GPIO_LED DEVICE MAJOR
#define GPIO_LED_MAJOR	97 //�����豸��

// define LED
#define LED1	(GPF4)  //�����ĸ�LED�Ƶ�I/O�˿�
#define LED2	(GPF5)
#define LED3	(GPF6)
#define LED4	(GPF7)

//define LED STATUS 
#define LED_ON  0 //����LED�Ƶ�״̬
#define LED_OFF 1

// define cmd
#define ctl_GPIO_LED1 1

struct file_operations GPIO_LED_ctl_ops ={

	open:		GPIO_LED_open,
	read:		GPIO_LED_read,
	write:		GPIO_LED_write,
	ioctl:		GPIO_LED_ioctl,
	release:	GPIO_LED_release,
};

// ------------------- READ ------------------------
ssize_t GPIO_LED_read (struct file * file ,char * buf, size_t count, loff_t * f_ops)
{
	return count;
}	

// ------------------- WRITE -----------------------
ssize_t GPIO_LED_write (struct file * file ,const char * buf, size_t count, loff_t * f_ops)
{
	return count;
}	

// ------------------- IOCTL -----------------------
ssize_t GPIO_LED_ioctl (struct inode * inode ,struct file * file, unsigned int cmd, long data)
{
	switch (cmd)
        {
		case LED_ON : { GPFDAT =(GPFDAT & ~(0xf<<4)) | ((~0x0f & 0xf)<<4); break;}
		case LED_OFF: { GPFDAT =(GPFDAT & ~(0xf<<4)) | ((~0x00 & 0xf)<<4); break;}
                default :
                        {printk ("lcd control : no cmd run  [ --kernel-- ]\n"); return (-EINVAL);}
        }
	return 0;
	
}

// ------------------- OPEN ------------------------
ssize_t GPIO_LED_open (struct inode * inode ,struct file * file)
{
	MOD_INC_USE_COUNT;
	
	return 0;
}	

// ------------------- RELEASE/CLOSE ---------------
ssize_t GPIO_LED_release (struct inode  * inode ,struct file * file)
{
	MOD_DEC_USE_COUNT;

	return 0;
}
// -------------------------------------------------

// ------------------- INIT ------------------------
static int GPIO_LED_CTL_init(void)
{
    int ret = -ENODEV;
	

	printk("--------------------------------------------\n\n");
	// init GPIO
	//=== PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
    //Binary :  01      01 ,  01     01  ,     10       10  , 10     10
    GPFCON = 0x55aa; // ���ö˿�ΪI/O���ģʽ
    GPFUP  = 0xff;     // �ر���������
	GPFDAT =0xf;//��ʼֵΪ�ߵ�ƽϨ��LED��

	ret = devfs_register_chrdev(GPIO_LED_MAJOR, "gpio_led_ctl", &GPIO_LED_ctl_ops);

	if( ret < 0 )
	{
		printk (" S3C2410: init_module failed with %d\n", ret);	
		return ret;
	}
	else
	{
		printk("S3C2410 gpio_led_driver register success!!! \n");
	}


	return ret;
}


static int __init S3C2410_GPIO_LED_CTL_init(void) 
{
    int  ret = -ENODEV;
     	
    ret = GPIO_LED_CTL_init();
    if (ret)
      return ret;
    return 0;
}

static void __exit cleanup_GPIO_LED_ctl(void)
{
	devfs_unregister_chrdev (GPIO_LED_MAJOR, "gpio_led_ctl" );
	
}
module_init(S3C2410_GPIO_LED_CTL_init);
module_exit(cleanup_GPIO_LED_ctl);

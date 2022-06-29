/*======================================================================
    A globalmem driver as an example of char device drivers

    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/



#include <linux/version.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>

//  error: implicit declaration of function `kfree`
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 19, 0)
#include <asm/system.h>
#else
#include <linux/slab.h>
#endif

#include <asm/uaccess.h>

#define GLOBALMEM_SIZE      0x1000            /*  ȫ���ڴ����4K�ֽ�          */
#define MEM_CLEAR           0x1               /*  ��0ȫ���ڴ�                 */
#define GLOBALMEM_MAJOR     300               /*  Ԥ���globalmem�����豸��   */


/*  globalmem���豸��   */
static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);


/*  globalmem�豸�ṹ�� */
struct globalmem_dev
{

    struct cdev cdev;                         /*  cdev�ṹ��  */
    unsigned char mem[GLOBALMEM_SIZE];        /*  ȫ���ڴ�    */

};

struct globalmem_dev *globalmem_devp;         /*  �豸�ṹ��ָ��    */


/*  �ļ��򿪺���    */
int globalmem_open(struct inode *inode, struct file *filp)
{
    /*    ���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ��  */
    filp->private_data = globalmem_devp;

    return 0;
}

/*  �ļ��ͷź���    */
int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
/* ioctl�豸���ƺ��� */
static int globalmem_ioctl(
        struct inode *inodep,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long globalmem_compat_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    struct inode *inode = inode = file_inode(filp);
#endif

    struct globalmem_dev *dev = filp->private_data;/*����豸�ṹ��ָ��*/

    switch (cmd)
    {
        case MEM_CLEAR:
        {
            memset(dev->mem, 0, GLOBALMEM_SIZE);
            printk(KERN_INFO "globalmem is set to zero\n");
            break;
        }

        default:
        {
            return  - EINVAL;
        }
    }

    return 0;
}

/*������*/
static ssize_t globalmem_read(
        struct file *filp,
        char __user *buf,
        size_t      size,
        loff_t      *ppos)
{
    unsigned long           p =  *ppos;                /*   �ļ�ָ��ƫ�� once = 0, twice = GLOBALMEM_SIZE  */
    unsigned int            count = size;              /*   GLOBALMEM_SIZE */
    int                     ret = 0;
    struct globalmem_dev    *dev = filp->private_data; /*����豸�ṹ��ָ��*/

    /*  �����ͻ�ȡ��Ч��д����  */
    if (p >= GLOBALMEM_SIZE)
    {
        printk("*ppos = %d\n", p);
        /*
         * https://zhidao.baidu.com/question/136829890072623445.html
         * http://bbs.chinaunix.net/thread-1961861-1-1.html
         * http://blog.csdn.net/qiaoliang328/article/details/4874238
         * Fix bug when `cat /dev/ XXX`
         *
         * return count ? 0 : -ENXIO;  �������㷵��-ENXIO, ��No such device or address
         *
         * ����cat����read�豸����, ������ֵ�Ǵ���0��ʱ��,
         * cat�������open�豸, Ȼ�����read����(ÿ�ζ�4096�ֽ�)
         * ���������е�һ��
         */
        return count ? -ENXIO : 0;
    }

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }

    /*  �ں˿ռ�->�û��ռ�
     *
     *  ��dev->mem��ƫ��pΪ��ʼ���ڴ����������, ����count���ֽڵ��û���buf��
     *  */
    if (copy_to_user(buf, (void*)(dev->mem + p), count))
    {
        ret =  - EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
    }

    return ret;
}

/*д����*/
static ssize_t globalmem_write(
        struct file         *filp,
        const char __user   *buf,
        size_t              size,
        loff_t              *ppos)
{
    unsigned long p =  *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; /*����豸�ṹ��ָ��*/

    /*  �����ͻ�ȡ��Ч��д����  */
    if (p >= GLOBALMEM_SIZE)
    {
        return count ?  - ENXIO: 0;
    }

    if (count > GLOBALMEM_SIZE - p)
    {
        count = GLOBALMEM_SIZE - p;
    }

    /*  �û��ռ�->�ں˿ռ�*/
    if (copy_from_user(dev->mem + p, buf, count) != 0)
    {
        ret =  - EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        printk(KERN_INFO "written %d bytes(s) from %d\n", count, p);
    }

    return ret;
}

/* seek�ļ���λ���� */
static loff_t globalmem_llseek(
        struct file *filp,
        loff_t      offset,
        int         orig)
{
    loff_t ret = 0;
    switch (orig)
    {
        case 0:   /*����ļ���ʼλ��ƫ��*/
            if (offset < 0)
            {
                ret =  - EINVAL;
                break;
            }

            if ((unsigned int)offset > GLOBALMEM_SIZE)
            {
                ret =  - EINVAL;
                break;
            }

            filp->f_pos = (unsigned int)offset;

            ret = filp->f_pos;

            break;

        case 1:   /*����ļ���ǰλ��ƫ��*/
            if ((filp->f_pos + offset) > GLOBALMEM_SIZE)
            {
                ret =  - EINVAL;
                break;
            }
            if ((filp->f_pos + offset) < 0)
            {
                ret =  - EINVAL;
                break;
            }

            filp->f_pos += offset;
            ret = filp->f_pos;

            break;

        default:

            ret =  - EINVAL;
            break;
    }

    return ret;
}

/*�ļ������ṹ��*/
static const struct file_operations globalmem_fops =
{
    .owner = THIS_MODULE,
    .llseek = globalmem_llseek,
    .read = globalmem_read,
    .write = globalmem_write,

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = globalmem_ioctl,
#else
    //.unlocked_ioctl = globalmem_ioctl,
    .compat_ioctl = globalmem_compat_ioctl,
#endif

    .open = globalmem_open,
    .release = globalmem_release,
};

/*��ʼ����ע��cdev*/
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err, devno = MKDEV(globalmem_major, index);

    cdev_init(&dev->cdev, &globalmem_fops);

    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &globalmem_fops;

    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
    }
}

/*  �豸����ģ����غ���    */
int globalmem_init(void)
{
    int result;
    dev_t devno = MKDEV(globalmem_major, 0);

    /*  �����豸��  */
    if (globalmem_major != 0)
    {
        result = register_chrdev_region(devno, 1, "globalmem");
        printk(KERN_INFO "register char device drivers [globalmem], MAJOR = %d\n", globalmem_major);
    }
    else  /* ��̬�����豸�� */
    {
        result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
        globalmem_major = MAJOR(devno);
        printk(KERN_INFO "alloc char device drivers [globalmem], MAJOR = %d\n", globalmem_major);
    }

    if (result < 0)
    {
        return result;
    }

    /* ��̬�����豸�ṹ����ڴ�*/
    globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp)    /*  ����ʧ��    */
    {
        result =  - ENOMEM;
        unregister_chrdev_region(devno, 1);

        return result;
    }
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

    globalmem_setup_cdev(globalmem_devp, 0);

    return 0;
}

/*ģ��ж�غ���*/
void globalmem_exit(void)
{
    cdev_del(&globalmem_devp->cdev);   /*ע��cdev*/

    kfree(globalmem_devp);     /*�ͷ��豸�ṹ���ڴ�*/

    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); /*�ͷ��豸��*/
}


//Driver Information
#define DRIVER_VERSION  "1.0.0"
#define DRIVER_AUTHOR   "Gatieme @ AderStep Inc..."
#define DRIVER_DESC     "Linux input module for Elo MultiTouch(MT) devices"
#define DRIVER_LICENSE  "Dual BSD/GPL"

// Kernel Module Information
MODULE_VERSION(DRIVER_VERSION);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);


module_init(globalmem_init);
module_exit(globalmem_exit);

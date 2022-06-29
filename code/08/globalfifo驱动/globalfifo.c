/*======================================================================
      A globalfifo driver as an example of char device drivers
      This example is to introduce poll,blocking and non-blocking access

      The initial developer of the original code is Baohua Song
      <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/version.h>
//#include <linux/printk.h>

#include <asm/io.h>

//#include <asm/system.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <linux/poll.h>

#define GLOBALFIFO_SIZE	    0x1000	    /*  ȫ��fifo���4K�ֽ�          */
#define FIFO_CLEAR          0x1         /*  ��0ȫ���ڴ�ĳ���           */
#define GLOBALFIFO_MAJOR    253         /*  Ԥ���globalfifo�����豸��  */

static int globalfifo_major = GLOBALFIFO_MAJOR;

/*  globalfifo�豸�ṹ��    */
struct globalfifo_dev
{
    struct cdev           cdev;                       /*  cdev�ṹ��              */
    unsigned int          current_len;                /*  fifo��Ч���ݳ���        */
    unsigned char         mem[GLOBALFIFO_SIZE];       /*  ȫ���ڴ�                */
    struct semaphore      sem;                        /*  ���������õ��ź���      */
    wait_queue_head_t     r_wait;                     /*  �������õĵȴ�����ͷ    */
    wait_queue_head_t     w_wait;                     /*  ����д�õĵȴ�����ͷ    */
};

struct globalfifo_dev *globalfifo_devp;             /*  �豸�ṹ��ָ��          */

/*  �ļ��򿪺���    */
int globalfifo_open(struct inode *inode, struct file *filp)
{
      /*���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ��*/
      filp->private_data = globalfifo_devp;

      return 0;
}

/*�ļ��ͷź���*/
int globalfifo_release(struct inode *inode, struct file *filp)
{
      return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
/* ioctl�豸���ƺ��� */
static int globalfifo_ioctl(
          struct inode *inodep,
          struct file *filp,
          unsigned int cmd,
          unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *, unsigned int cmd, unsigned long arg)
static long globalfifo_unlocked_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
    struct inode *inode = inode = file_inode(filp);
#endif

    struct globalfifo_dev *dev = filp->private_data;  /*����豸�ṹ��ָ��*/

    switch (cmd)
    {
      case FIFO_CLEAR:
      	down(&dev->sem); //����ź���
        dev->current_len = 0;
        memset(dev->mem,0,GLOBALFIFO_SIZE);
        up(&dev->sem); //�ͷ��ź���

        printk(KERN_INFO "globalfifo is set to zero\n");
        break;

      default:
        return  - EINVAL;
    }
    return 0;
}

static unsigned int globalfifo_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    struct globalfifo_dev *dev = filp->private_data; /*����豸�ṹ��ָ��*/

    down(&dev->sem);

    poll_wait(filp, &dev->r_wait, wait);
    poll_wait(filp, &dev->w_wait, wait);
    /*fifo�ǿ�*/
    if (dev->current_len != 0)
    {
        mask |= POLLIN | POLLRDNORM; /*��ʾ���ݿɻ��*/
    }
    /*fifo����*/
    if (dev->current_len != GLOBALFIFO_SIZE)
    {
        mask |= POLLOUT | POLLWRNORM; /*��ʾ���ݿ�д��*/
    }

    up(&dev->sem);
    return mask;
}


/*globalfifo������*/
static ssize_t globalfifo_read(
          struct file *filp,
          char __user *buf,
          size_t      count,
          loff_t      *ppos)
{
    int                     ret;
    struct  globalfifo_dev  *dev = filp->private_data; //����豸�ṹ��ָ��

    DECLARE_WAITQUEUE(wait, current);       //  ����ȴ�����


    down(&dev->sem);                        //  ����ź���
    add_wait_queue(&dev->r_wait, &wait);    //  ������ȴ�����ͷ

    /* �ȴ�FIFO�ǿ� */
    if (dev->current_len == 0)
    {
        if (filp->f_flags &O_NONBLOCK)
        {
            ret =  - EAGAIN;
            goto out;
        }

        __set_current_state(TASK_INTERRUPTIBLE);    //�ı����״̬Ϊ˯��

        up(&dev->sem);

        schedule( ); /*  ������������ִ��  */

        if (signal_pending(current))  /*  �������Ϊ�źŻ���  */
        {
            ret =  - ERESTARTSYS;
            goto out2;
        }

        down(&dev->sem);
    }

    /* �������û��ռ� */
    if (count > dev->current_len)
    {
        count = dev->current_len;
    }

    if (copy_to_user(buf, dev->mem, count))
    {
        ret =  - EFAULT;
        goto out;
    }
    else
    {
        memcpy(dev->mem, dev->mem + count, dev->current_len - count); //fifo����ǰ��
        dev->current_len -= count; //��Ч���ݳ��ȼ���
        printk(KERN_INFO "read %d bytes(s),current_len:%d\n", count, dev->current_len);

        wake_up_interruptible(&dev->w_wait); //����д�ȴ�����

        ret = count;
    }


out     :
    up(&dev->sem); //�ͷ��ź���
out2    :
    remove_wait_queue(&dev->w_wait, &wait); //�Ӹ����ĵȴ�����ͷ�Ƴ�
    set_current_state(TASK_RUNNING);

    return ret;
}


/*globalfifoд����*/
static ssize_t globalfifo_write(struct file *filp, const char __user *buf,
    size_t count, loff_t *ppos)
{
    struct globalfifo_dev *dev = filp->private_data; //����豸�ṹ��ָ��
    int ret;
    DECLARE_WAITQUEUE(wait, current); //����ȴ�����

    down(&dev->sem); //��ȡ�ź���
    add_wait_queue(&dev->w_wait, &wait); //����д�ȴ�����ͷ

    /* �ȴ�FIFO���� */
    if (dev->current_len == GLOBALFIFO_SIZE)
    {
        if (filp->f_flags &O_NONBLOCK)
        //����Ƿ���������
        {
            ret =  - EAGAIN;
            goto out;
        }
        __set_current_state(TASK_INTERRUPTIBLE); //�ı����״̬Ϊ˯��
        up(&dev->sem);

        schedule(); /*  ������������ִ��  */
        if (signal_pending(current))    /*  �������Ϊ�źŻ���  */
        {
            ret =  - ERESTARTSYS;
            goto out2;
        }

        down(&dev->sem); //����ź���
    }

    /*���û��ռ俽�����ں˿ռ�*/
    if (count > GLOBALFIFO_SIZE - dev->current_len)
        count = GLOBALFIFO_SIZE - dev->current_len;

    if (copy_from_user(dev->mem + dev->current_len, buf, count))
    {
        ret =  - EFAULT;
        goto out;
    }
    else
    {
        dev->current_len += count;
        printk(KERN_INFO "written %d bytes(s),current_len:%d\n",
                count, dev->current_len);

        wake_up_interruptible(&dev->r_wait); //���Ѷ��ȴ�����

        ret = count;
    }

out:
    up(&dev->sem); //�ͷ��ź���

out2:
    remove_wait_queue(&dev->w_wait, &wait); //�Ӹ����ĵȴ�����ͷ�Ƴ�
    set_current_state(TASK_RUNNING);

    return ret;
}


/*�ļ������ṹ��*/
static const struct file_operations globalfifo_fops =
{
    .owner = THIS_MODULE,
    .read = globalfifo_read,
    .write = globalfifo_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = globalfifo_ioctl,
#else
    .unlocked_ioctl = globalfifo_unlocked_ioctl,
#endif
    .poll = globalfifo_poll,
    .open = globalfifo_open,
    .release = globalfifo_release,
};

/*��ʼ����ע��cdev*/
static void globalfifo_setup_cdev(struct globalfifo_dev *dev, int index)
{
    int err, devno = MKDEV(globalfifo_major, index);

    cdev_init(&dev->cdev, &globalfifo_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &globalfifo_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/*�豸����ģ����غ���*/
int globalfifo_init(void)
{
    int ret;
    dev_t devno = MKDEV(globalfifo_major, 0);

    /* �����豸��*/
    if (globalfifo_major)
        ret = register_chrdev_region(devno, 1, "globalfifo");
    else  /* ��̬�����豸�� */
    {
        ret = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
        globalfifo_major = MAJOR(devno);
    }
    if (ret < 0)
      return ret;
    /* ��̬�����豸�ṹ����ڴ�*/
    globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp)    /*����ʧ��*/
    {
        ret =  - ENOMEM;
        goto fail_malloc;
    }

    memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

    globalfifo_setup_cdev(globalfifo_devp, 0);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 36) && !defined(init_MUTEX)
    sema_init(&globalfifo_devp->sem, 1);
#else
    init_MUTEX(&globalfifo_devp->sem);   /*��ʼ���ź���*/
#endif
    init_waitqueue_head(&globalfifo_devp->r_wait); /*��ʼ�����ȴ�����ͷ*/
    init_waitqueue_head(&globalfifo_devp->w_wait); /*��ʼ��д�ȴ�����ͷ*/

    return 0;

    fail_malloc: unregister_chrdev_region(devno, 1);
    return ret;
}


/*ģ��ж�غ���*/
void globalfifo_exit(void)
{
    cdev_del(&globalfifo_devp->cdev);   /*ע��cdev*/
    kfree(globalfifo_devp);     /*�ͷ��豸�ṹ���ڴ�*/
    unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1); /*�ͷ��豸��*/
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalfifo_major, int, S_IRUGO);

module_init(globalfifo_init);
module_exit(globalfifo_exit);

/*======================================================================
    A VirtualDisk driver as an example of char device drivers

======================================================================*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/version.h>


#define VIRTUALDISK_SIZE	0x2000	/*ȫ���ڴ����8K�ֽ�*/
#define MEM_CLEAR 0x1  /*ȫ���ڴ�����*/
#define PORT1_SET 0x2  /*��port1�˿�����*/
#define PORT2_SET 0x3  /*��port2�˿�����*/
/*
#define VirtualDisk_MAGIC 100
#define MEM_CLEAR _IO(VirtualDisk_MAGIC,0)
#define PORT1_SET _IO(VirtualDisk_MAGIC,1)
#define PORT2_SET _IO(VirtualDisk_MAGIC,2)
*/
#define VIRTUALDISK_MAJOR 200    /*Ԥ���VirtualDisk�����豸��Ϊ200*/

static int VirtualDisk_major = VIRTUALDISK_MAJOR;
/*VirtualDisk�豸�ṹ��*/
struct VirtualDisk
{
  struct cdev cdev; /*cdev�ṹ��*/
  unsigned char mem[VIRTUALDISK_SIZE]; /*ȫ���ڴ�8K*/
  int port1; /*������ͬ���͵Ķ˿�*/
  long port2;
  long count;  /*��¼�豸Ŀǰ�������豸��*/
};

struct VirtualDisk *Virtualdisk_devp; /*�豸�ṹ��ָ��*/
/*�ļ��򿪺���*/
int VirtualDisk_open(struct inode *inode, struct file *filp)
{
  struct VirtualDisk *devp = NULL;

  /*���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ��*/
  filp->private_data = Virtualdisk_devp;
  devp = filp->private_data;/*����豸�ṹ��ָ��*/
  devp->count++;/*�����豸�򿪴���*/

  return 0;
}
/*�ļ��ͷź���*/
int VirtualDisk_release(struct inode *inode, struct file *filp)
{
  struct VirtualDisk *devp = filp->private_data;/*����豸�ṹ��ָ��*/
  devp->count--;/*�����豸�򿪴���*/
  return 0;
}


/* ioctl�豸���ƺ��� */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
static int VirtualDisk_ioctl(
        struct inode *inodep,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long VirtualDisk_unlocked_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
  //struct inode *inode = filp->f_dentry->d_inode;
  struct inode *inode = inode = file_inode(filp);
#endif
  struct VirtualDisk *devp = filp->private_data;/*����豸�ṹ��ָ��*/

  switch (cmd)
  {
    case MEM_CLEAR:/*�豸�ڴ�����*/
      memset(devp->mem, 0, VIRTUALDISK_SIZE);
      printk(KERN_INFO "VirtualDisk is set to zero\n");
      break;
	case PORT1_SET:/*���˿�1��0*/
	  devp->port1=0;
	  break;
	case PORT2_SET:/*���˿�2��0*/
	  devp->port2=0;
	  break;
    default:
      return  - EINVAL;
  }
  return 0;
}

/*������*/
static ssize_t VirtualDisk_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
  unsigned long p =  *ppos; /*��¼�ļ�ָ��ƫ��λ��*/
  unsigned int count = size;/*��¼��Ҫ��ȡ���ֽ���*/
  int ret = 0;/*����ֵ*/
  struct VirtualDisk *devp = filp->private_data; /*����豸�ṹ��ָ��*/

  printk("p = %ld\n", p);
  /*�����ͻ�ȡ��Ч�Ķ�����*/
  if (p >= VIRTUALDISK_SIZE)  /*Ҫ��ȡ��ƫ�ƴ����豸���ڴ�ռ�*/
    return count ?  - ENXIO: 0;/*��ȡ��ַ����*/
  if (count > VIRTUALDISK_SIZE - p)/*Ҫ��ȡ���ֽڴ����豸���ڴ�ռ�*/
    count = VIRTUALDISK_SIZE - p;/*��Ҫ��ȡ���ֽ�����Ϊʣ����ֽ���*/
  /*�ں˿ռ�->�û��ռ佻������*/
  if (copy_to_user(buf, (void*)(devp->mem + p), count))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;
    printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  }
  return ret;
}

/*д����*/
static ssize_t VirtualDisk_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos; /*��¼�ļ�ָ��ƫ��λ��*/
  int ret = 0;  /*����ֵ*/
  unsigned int count = size;/*��¼��Ҫд����ֽ���*/
  struct VirtualDisk *devp = filp->private_data; /*����豸�ṹ��ָ��*/

  /*�����ͻ�ȡ��Ч��д����*/
  if (p >= VIRTUALDISK_SIZE)/*Ҫд���ƫ�ƴ����豸���ڴ�ռ�*/
    return count ?  - ENXIO: 0;/*д���ַ����*/
  if (count > VIRTUALDISK_SIZE - p)/*Ҫд����ֽڴ����豸���ڴ�ռ�*/
    count = VIRTUALDISK_SIZE - p;/*��Ҫд����ֽ�����Ϊʣ����ֽ���*/

  /*�û��ռ�->�ں˿ռ�*/
  if (copy_from_user(devp->mem + p, buf, count))
    ret =  - EFAULT;
  else
  {
    *ppos += count;/*����ƫ��λ��*/
    ret = count;/*����ʵ�ʵ�д���ֽ���*/
    printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
  }
  return ret;
}

/* seek�ļ���λ���� */
static loff_t VirtualDisk_llseek(struct file *filp, loff_t offset, int orig)
{
  loff_t ret = 0;/*���ص�λ��ƫ��*/
  switch (orig)
  {
    case SEEK_SET:   /*����ļ���ʼλ��ƫ��*/
      if (offset < 0)/*offset���Ϸ�*/
      {
        ret =  - EINVAL;		/*��Ч��ָ��*/
        break;
      }
      if ((unsigned int)offset > VIRTUALDISK_SIZE)/*ƫ�ƴ����豸�ڴ�*/
      {
        ret =  - EINVAL;		/*��Ч��ָ��*/
        break;
      }
      filp->f_pos = (unsigned int)offset;	/*�����ļ�ָ��λ��*/
      ret = filp->f_pos;/*���ص�λ��ƫ��*/
      break;
    case SEEK_CUR:   /*����ļ���ǰλ��ƫ��*/
      if ((filp->f_pos + offset) > VIRTUALDISK_SIZE)/*ƫ�ƴ����豸�ڴ�*/
      {
        ret =  - EINVAL;/*��Ч��ָ��*/
        break;
      }
      if ((filp->f_pos + offset) < 0)/*ָ�벻�Ϸ�*/
      {
        ret =  - EINVAL;/*��Ч��ָ��*/
        break;
      }
      filp->f_pos += offset;/*�����ļ�ָ��λ��*/
      ret = filp->f_pos;/*���ص�λ��ƫ��*/
      break;
    default:
      ret =  - EINVAL;/*��Ч��ָ��*/
      break;
  }
  return ret;
}

/*�ļ������ṹ��*/
static const struct file_operations VirtualDisk_fops =
{
  .owner = THIS_MODULE,
  .llseek = VirtualDisk_llseek,/*��λƫ��������*/
  .read = VirtualDisk_read,/*���豸����*/
  .write = VirtualDisk_write,/*д�豸����*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
  .ioctl = VirtualDisk_ioctl,/*���ƺ���*/
#else
  .compat_ioctl = VirtualDisk_unlocked_ioctl,
#endif
  .open = VirtualDisk_open,/*���豸����*/
  .release = VirtualDisk_release,/*�ͷ��豸����*/
};

/*��ʼ����ע��cdev*/
static void VirtualDisk_setup_cdev(struct VirtualDisk *dev, int minor)
{
  int err;
  dev_t devno = MKDEV(VirtualDisk_major, minor);/*�����豸��*/

  cdev_init(&dev->cdev, &VirtualDisk_fops);/*��ʼ��cdev�豸*/
  dev->cdev.owner = THIS_MODULE;/*ʹ�����������ڸ�ģ��*/
  dev->cdev.ops = &VirtualDisk_fops;/*cdev����file_operationsָ��*/

  err = cdev_add(&dev->cdev, devno, 1);/*��cdevע�ᵽϵͳ��*/

  if (err)
    printk(KERN_NOTICE "Error in cdev_add()\n");
}
/*�豸����ģ����غ���*/
int VirtualDisk_init(void)
{
  int result;
  dev_t devno = MKDEV(VirtualDisk_major, 0); /*�����豸��*/

  /* �����豸��*/
  if (VirtualDisk_major)  /* �����Ϊ0����̬����*/
    result = register_chrdev_region(devno, 1, "VirtualDisk");
  else  /* ��̬�����豸�� */
  {
    result = alloc_chrdev_region(&devno, 0, 1, "VirtualDisk");
    VirtualDisk_major = MAJOR(devno);/* �������豸���еõ����豸�� */
  }
  if (result < 0)
    return result;

  /* ��̬�����豸�ṹ����ڴ�*/
  Virtualdisk_devp = kmalloc(sizeof(struct VirtualDisk), GFP_KERNEL);
  if (!Virtualdisk_devp)    /*����ʧ��*/
  {
    result =  - ENOMEM;
    goto fail_kmalloc;
  }
  memset(Virtualdisk_devp, 0, sizeof(struct VirtualDisk));/*���ڴ�����*/
  /*��ʼ���������cdev�ṹ��*/
  VirtualDisk_setup_cdev(Virtualdisk_devp, 0);
  return 0;

  fail_kmalloc:
  unregister_chrdev_region(devno, 1);
  return result;
}

/*ģ��ж�غ���*/
void VirtualDisk_exit(void)
{
  cdev_del(&Virtualdisk_devp->cdev);   /*ע��cdev*/
  kfree(Virtualdisk_devp);     /*�ͷ��豸�ṹ���ڴ�*/
  unregister_chrdev_region(MKDEV(VirtualDisk_major, 0), 1); /*�ͷ��豸��*/
}

MODULE_AUTHOR("Zheng Qiang");
MODULE_LICENSE("Dual BSD/GPL");

module_param(VirtualDisk_major, int, S_IRUGO);

module_init(VirtualDisk_init);
module_exit(VirtualDisk_exit);

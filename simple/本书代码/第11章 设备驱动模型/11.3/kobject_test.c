#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
 
 
void kobject_test_release(struct kobject *kobject);/*�ͷ�kobject�ṹ��ĺ���*/
/*�����Եĺ���*/
ssize_t kobject_test_show(struct kobject *kobject, struct attribute *attr,char *buf);
/*д���Եĺ���*/
ssize_t kobject_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count);
/*������һ����Ϊkobject_test�����Զ�д������*/
struct attribute test_attr = {
	.name = "kobject_test_attr",/*������*/
	.mode = S_IRWXUGO,/*����Ϊ�ɶ���д*/
};
/*��kobjectֻ��һ������*/
static struct attribute *def_attrs[] = {
	&test_attr,
	NULL,
};
struct sysfs_ops obj_test_sysops =
{
	.show = kobject_test_show,/*���Զ�����*/
	.store = kobject_test_store,/*����д����*/
};

struct kobj_type ktype = 
{
	.release = kobject_test_release,/*�ͷź���*/
	.sysfs_ops=&obj_test_sysops,/*���ԵĲ�������*/
	.default_attrs=def_attrs,/*Ĭ������*/
};
void kobject_test_release(struct kobject *kobject)
{
	printk("kobject_test: kobject_test_release() .\n");/*��ֻ��һ���������ӣ�ʵ�ʵĴ���Ҫ���Ӻܶ�*/
}
/*�ú���������ȡһ�����Ե�����*/
ssize_t kobject_test_show(struct kobject *kobject, struct attribute *attr,char *buf)
{
	printk("call kobject_test_show().\n");/*������Ϣ*/
	printk("attrname:%s.\n", attr->name);/*��ӡ���Ե�����*/
	sprintf(buf,"%s\n",attr->name);/*���������ִ����buf�У������û��ռ�*/
	return strlen(attr->name)+2;
}
/*�ú�������д��һ�����Ե�ֵ*/
ssize_t kobject_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count)
{
	printk("call kobject_test_store().\n");/*������Ϣ*/
	printk("write: %s\n",buf);/*���Ҫ�������Ϣ*/
	strcpy(attr->name,buf);
	return count;
}

struct kobject kobj;/*Ҫ��ӵ�kobject�ṹ*/
static int kobject_test_init()
{
	printk("kboject test_init().\n");
	kobject_init_and_add(&kobj,&ktype,NULL,"kobject_test");/*��ʼ�������kobject���ں���*/
	return 0;
}
static int kobject_test_exit()
{
	printk("kobject test exit.\n");
	kobject_del(&kobj);/*ɾ��kobject*/
	return 0;
}
module_init(kobject_test_init);
module_exit(kobject_test_exit);

MODULE_AUTHOR("Zheng Qiang");
MODULE_LICENSE("Dual BSD/GPL");

2410 nand flash�������ļ�
#include <linux/slab.h>                            
#include <linux/module.h>                          
#include <linux/mtd/mtd.h>                         
#include <linux/mtd/nand.h>                        
#include <linux/mtd/compatmac.h>                   
#include <linux/irq.h>                             
#include <asm/io.h>                                
#include <asm/hardware.h>                          
#include <asm/sizes.h>                             
#include <linux/compiler.h>                        
#include <linux/delay.h>                           
#include <linux/interrupt.h>                       
#include <linux/sched.h>                           
                                                      
static struct mtd_info * smc _mtd = NULL;        
                                                   
#include <linux/mtd/partitions.h>                  
                                                   
  //��Ӧ�ڿ������NAND Flash����
static struct mtd_partition smc_partitions[] = {
	{
		name:		"vivi",
		size:		0x00030000,
		offset:		0x0,
		mask_flags:	MTD_WRITEABLE,
	},{
		name:		"kernel",
		size:		0x000d0000,
		offset:		0x00030000,
		mask_flags:	MTD_WRITEABLE,	//force read-only 
	}, {
		name:		"root",
		size:		0x03200000,
		offset:		0x00100000,
		mask_flags:	MTD_WRITEABLE,  // force read-only
	},{
		name:           "user_file",
                size:           0x00A00000,
                offset:         0x03300000,
	}
};
                                                                           
static void smc_hwcontrol(int cmd) {
    switch (cmd) {
    case NAND_CTL_SETNCE:	NFCONF &= ~NFCONF_nFCE_HIGH; break;
    case NAND_CTL_CLRNCE:	NFCONF |= NFCONF_nFCE_HIGH; break;
    case NAND_CTL_SETCLE:	break;
    case NAND_CTL_CLRCLE:	break;
    case NAND_CTL_SETALE:	break;
    case NAND_CTL_CLRALE:	break;
    case NAND_CTL_DAT_IN:	break;
    case NAND_CTL_DAT_OUT:	break;
    }
}                                 
 static void write_cmd(u_char val) {
    NFCMD = (u_char) val;
}

static void write_addr(u_char val) {
    NFADDR = (u_char) val;
}

static u_char read_data(void) {
    return (u_char) NFDATA;
}

static void write_data(u_char val) {
    NFDATA = (u_char) val;
}

static void wait_for_ready(void) {
    while (!(NFSTAT & NFSTAT_RnB)) {
      /* æ */
      udelay(10);
    }
}

inline int smc_insert(struct nand_chip *this) {
    /* Scan to find existance of the device */
    if (smc_scan (s3c2410_mtd)) {
      return -ENXIO;
    }
    /* Ϊ�ڲ������������ڴ� */
    this->data_buf = kmalloc(sizeof(u_char) * 
			     (s3c2410_mtd->oobblock + s3c2410_mtd->oobsize), 
			     GFP_KERNEL);
    if (!this->data_buf) {
      printk ("Unable to allocate NAND data buffer for S3C2410.\n");
      this->data_buf = NULL;
      return -ENOMEM;
    }
#ifdef CONFIG_MTD_SMC_S3C2410_SMDK_PARTITION
    add_mtd_partitions(s3c2410_mtd, &smc_partitions, 
		       sizeof(smc_partitions)/sizeof(smc_partitions[0]));
//#else
    add_mtd_device(s3c2410_mtd);
#endif

    return 0;
}

inline void smc_remove(struct nand_chip *this) {
#if defined(CONFIG_MTD_SMC_S3C2410_SMDK_PARTITION) 
    del_mtd_partitions(s3c2410_mtd);
//#else
    del_mtd_device(s3c2410_mtd);
#endif

    s3c2410_mtd->size = 0;
    if (this->data_buf != NULL) {
      /* �ͷ����ݻ�����*/
      kfree (this->data_buf);
      this->data_buf = NULL;
    }
}

#ifdef CONFIG_PM
static unsigned long nfcon;
static int
s3c2410_smc_pm_callback(struct pm_dev *pm_dev, pm_request_t req, void *data)
{
	struct nand_chip *this = (struct nand_chip *)pm_dev->data;
	switch (req) {
		case PM_SUSPEND:
			nfcon = NFCONF;
			break;
		case PM_RESUME:
			NFCONF = nfcon;
			/* ʹ��оƬ-> ��λ -> �ȴ����ɶ� -> ����оƬ */
			this->hwcontrol(NAND_CTL_SETNCE);
			this->write_cmd(NAND_CMD_RESET);
			this->wait_for_ready();
			this->hwcontrol(NAND_CTL_CLRNCE);
			break;

	}
	return 0;
}
#endif
/*
 * ������
 */
int __init smc_s3c2410_init (void)
{
    struct nand_chip *this;
    u_int16_t nfconf;
    /*Ϊ MTD�豸�����ڴ� */
    s3c2410_mtd = kmalloc (sizeof(struct mtd_info) + sizeof (struct nand_chip),
			GFP_KERNEL);
    if (!s3c2410_mtd) {
      printk ("Unable to allocate S3C2410 NAND MTD device structure.\n");
      return -ENOMEM;
    }

    /* ȡ��ָ�����ݵ�ָ��*/
    this = (struct nand_chip *) (&s3c2410_mtd[1]);

    /* ��ʼ���ṹ */
    memset((char *) s3c2410_mtd, 0, sizeof(struct mtd_info));
    memset((char *) this, 0, sizeof(struct nand_chip));

      s3c2410_mtd->priv = this;

    /* �������������*/
    {
      nfconf = NFCONF;
      /* ʹ�����������*/
      nfconf |= NFCONF_FCTRL_EN;

      /* ��������ʱ�� */
      nfconf &= ~NFCONF_TWRPH1;   /* 0x0 */
      nfconf |= NFCONF_TWRPH0_3;  /* 0x3 */
      nfconf &= ~NFCONF_TACLS;    /* 0x0 */

      NFCONF = nfconf;
    }

    /* ����NAND��I/O��ַ */
    this->hwcontrol = smc_hwcontrol;
    this->write_cmd = write_cmd;
    this->write_addr = write_addr;
    this->read_data = read_data;
    this->write_data = write_data;
    this->wait_for_ready = wait_for_ready;

    /* ʹ��оƬ-> ��λ -> �ȴ����ɶ� -> ����оƬ */
    this->hwcontrol(NAND_CTL_SETNCE);
    this->write_cmd(NAND_CMD_RESET);
    this->wait_for_ready();
    this->hwcontrol(NAND_CTL_CLRNCE);
    smc_insert(this);
#ifdef CONFIG_PM
	smc_pm_dev = pm_register(PM_DEBUG_DEV, PM_SYS_MISC, s3c2410_smc_pm_callback);
	if (smc_pm_dev)
		smc_pm_dev->data = &s3c2410_mtd[1];
#endif

    return 0;
}
module_init(smc_s3c2410_init);

#ifdef MODULE
static void __exit smc_s3c2410_cleanup (void)
{
    struct nand_chip *this = (struct nand_chip *) &s3c2410_mtd[1];

    smc_remove(this);

    /* �ͷ�MTD�豸 */
    kfree (s3c2410_mtd);
}
module_exit(smc_s3c2410_cleanup);
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SMC Card on S3C2410 board");

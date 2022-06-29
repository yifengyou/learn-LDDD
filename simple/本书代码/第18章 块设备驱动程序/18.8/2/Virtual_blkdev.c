
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
//#include <linux/elevator.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>
#include <linux/hdreg.h>


#define VIRTUAL_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR
#define VIRTUAL_BLKDEV_DISKNAME "Virtual_blkdev"


#define VIRTUAL_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR /*���豸��*/
#define VIRTUAL_BLKDEV_DISKNAME "Virtual_blkdev"/*�豸��*/
#define VIRTUAL_BLKDEV_BYTES (8*1024*1024)/*�豸�Ĵ�СΪ8M*/
#define VIRTUAL_BLKDEV_MAXPARTITIONS (5)			/*�����4������*/

static struct request_queue *Virtual_blkdev_queue;/*�������ָ��*/
static struct gendisk *Virtual_blkdev_disk;/*ͨ�ô���*/
unsigned char Virtual_blkdev_data[VIRTUAL_BLKDEV_BYTES];/*8M�����Ծ�̬�ڴ�ռ�*/

static int Virtual_blkdev_getgeo(struct block_device *bdev,struct hd_geometry *geo)
{
	/*
	* capacity heads sectors cylinders
	* 0~16M 1 1 0~32768
	* 16M~512M 1 32 1024~32768
	* 512M~16G 32 32 1024~32768
	* 16G~... 255 63 2088~...
	*/
	if (VIRTUAL_BLKDEV_BYTES < 16 * 1024 * 1024) {
		geo->heads = 1;
		geo->sectors = 1;
	} else if (VIRTUAL_BLKDEV_BYTES < 512 * 1024 * 1024) {
		geo->heads = 1;
		geo->sectors = 32;
	} else if (VIRTUAL_BLKDEV_BYTES < 16ULL * 1024 * 1024 * 1024) {
		geo->heads = 32;
		geo->sectors = 32;
	} else {
		geo->heads = 255;
		geo->sectors = 63;
	}
	geo->cylinders = VIRTUAL_BLKDEV_BYTES >>9/geo->heads/geo->sectors;
	return 0;
}


static int Virtual_blkdev_make_request(struct request_queue *q, struct bio *bio)
{
	struct bio_vec *bvec;
	int i;
	void *dsk_mem;
	if ((bio->bi_sector << 9) + bio->bi_size > VIRTUAL_BLKDEV_BYTES) {
		printk(KERN_ERR VIRTUAL_BLKDEV_DISKNAME
		": bad request: block=%llu, count=%u\n",
		(unsigned long long)bio->bi_sector, bio->bi_size);
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
			bio_endio(bio, 0, -EIO);
		#else
			bio_endio(bio, -EIO);
		#endif
		return 0;
	}
	dsk_mem = Virtual_blkdev_data + (bio->bi_sector << 9);
	bio_for_each_segment(bvec, bio, i) {
		void *iovec_mem;
		switch (bio_rw(bio)) {
			case READ:
			case READA:
			iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
			memcpy(iovec_mem, dsk_mem, bvec->bv_len);
			kunmap(bvec->bv_page);
			break;
			case WRITE:
			iovec_mem = kmap(bvec->bv_page) + bvec->bv_offset;
			memcpy(dsk_mem, iovec_mem, bvec->bv_len);
			kunmap(bvec->bv_page);
			break;
			default:
			printk(KERN_ERR VIRTUAL_BLKDEV_DISKNAME
			": unknown value of bio_rw: %lu\n",
			bio_rw(bio));
			#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
			bio_endio(bio, 0, -EIO);
			#else
			bio_endio(bio, -EIO);
			#endif
			return 0;
		}
		dsk_mem += bvec->bv_len;
	}
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
		bio_endio(bio, bio->bi_size, 0);
	#else
		bio_endio(bio, 0);
	#endif
	return 0;
}


struct block_device_operations Virtual_blkdev_fops =
{
	.owner = THIS_MODULE,
	.getgeo = Virtual_blkdev_getgeo,
};

static int __init Virtual_blkdev_init(void)
{
    int ret;

	Virtual_blkdev_disk = alloc_disk(VIRTUAL_BLKDEV_MAXPARTITIONS);
	if (!Virtual_blkdev_disk) 
	{
		ret = -ENOMEM;
		goto err_alloc_disk;
	}
	Virtual_blkdev_queue = blk_alloc_queue(GFP_KERNEL);;
	if (!Virtual_blkdev_queue) 
	{
		ret = -ENOMEM;
		goto err_alloc_queue;
	}
	blk_queue_make_request(Virtual_blkdev_queue, Virtual_blkdev_make_request);
	
	strcpy(Virtual_blkdev_disk->disk_name, VIRTUAL_BLKDEV_DISKNAME);
	Virtual_blkdev_disk->major = VIRTUAL_BLKDEV_DEVICEMAJOR;
	Virtual_blkdev_disk->first_minor = 0;
	Virtual_blkdev_disk->fops = &Virtual_blkdev_fops;
	Virtual_blkdev_disk->queue = Virtual_blkdev_queue;
	set_capacity(Virtual_blkdev_disk, VIRTUAL_BLKDEV_BYTES>>9);
	add_disk(Virtual_blkdev_disk);
	return 0;
	err_alloc_queue:
		put_disk(Virtual_blkdev_disk);
	err_alloc_disk:
		return ret;
}
static void __exit Virtual_blkdev_exit(void)
{
	del_gendisk(Virtual_blkdev_disk);
	put_disk(Virtual_blkdev_disk);
	blk_cleanup_queue(Virtual_blkdev_queue);
}
module_init(Virtual_blkdev_init);
module_exit(Virtual_blkdev_exit);

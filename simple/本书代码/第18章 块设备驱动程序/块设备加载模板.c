static int __init Virtual_blkdev_init(void)
{
	int ret;
	static struct gendisk *xxx_disk; /*ͨ�ô��̽ṹ*/
	static struct request_queue *xxx_queue; /*�������*/
	xxx_disk = alloc_disk(1); /*����ͨ�ô���*/
	if (!xxx_disk) {/*����ͨ�ô���ʧ�ܣ��˳�ģ��*/
		ret = -ENOMEM;
		goto err_alloc_disk;
	}
	
	if(register_blkdev(xxx_MAJOR,"xxx"))/*ע���豸*/
	{
		ret=-EIO;
		goto err_alloc_disk;
	}
	
	xxx_queue = blk_init_queue(xx_request, NULL);/*������г�ʼ��*/
	if (!xxx_queue) {/*������г�ʼ��ʧ��*/
		ret = -ENOMEM;
		goto err_init_queue;
	}

	strcpy(xxx_disk->disk_name, XXX_DISKNAME);/*�趨�豸��*/
	xxx_disk->major =xxx_MAJOR;/*�������豸��*/
	xxx_disk->first_minor = 0;/*���ô��豸��*/
	xxx_disk->fops = &xxx_fops;/*���豸��������*/
	xxx_disk->queue = xxx_queue;/*�����������*/
	set_capacity(xxx_disk, xxx_BYTES>>9);/*�����豸����*/
	add_disk(xxx_disk);/*��������豸*/
	return 0;
	err_init_queue:/*���г�ʼ��ʧ��*/
		unregister_blkdev(xxx_MAJOR,"xxx");
	err_alloc_disk:/*�������ʧ��*/
		put_disk(xxx_disk);
		return ret;
}
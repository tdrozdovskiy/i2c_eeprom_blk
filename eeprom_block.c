/* Disk on EEPROM Driver */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/errno.h>

#include "eeprom_device.h"

#define EB_FIRST_MINOR 0
#define EB_MINOR_CNT 16

static u_int eb_major = 0;

/* 
 * The internal structure representation of our Device
 */
static struct eb_device
{
	/* Size is the size of the device (in sectors) */
	unsigned int size;
	/* For exclusive access to our request queue */
	spinlock_t lock;
	/* Our request queue */
	struct request_queue *eb_queue;
	/* This is kernel's representation of an individual disk device */
	struct gendisk *eb_disk;
} eb_dev;

static int eb_open(struct block_device *bdev, fmode_t mode)
{
	unsigned unit = iminor(bdev->bd_inode);

	printk(KERN_INFO "eb: Device is opened\n");
	printk(KERN_INFO "eb: Inode number is %d\n", unit);

	if (unit > EB_MINOR_CNT)
		return -ENODEV;
	return 0;
}

static int eb_close(struct gendisk *disk, fmode_t mode)
{
	printk(KERN_INFO "eb: Device is closed\n");
	return 0;
}

/* 
 * Actual Data transfer
 */
static int eb_transfer(struct request *req)
{
	//struct eb_device *dev = (struct eb_device *)(req->rq_disk->private_data);

	int dir = rq_data_dir(req);
	sector_t start_sector = blk_rq_pos(req);
	unsigned int sector_cnt = blk_rq_sectors(req);

	struct bio_vec *bv;
	struct req_iterator iter;

	sector_t sector_offset;
	unsigned int sectors;
	u8 *buffer;

	int ret = 0;

	//printk(KERN_DEBUG "eb: Dir:%d; Sec:%lld; Cnt:%d\n", dir, start_sector, sector_cnt);

	sector_offset = 0;
	rq_for_each_segment(bv, req, iter)
	{
		buffer = page_address(bv->bv_page) + bv->bv_offset;
		if (bv->bv_len % EB_SECTOR_SIZE != 0)
		{
			printk(KERN_ERR "eb: Should never happen: "
				"bio size (%d) is not a multiple of EB_SECTOR_SIZE (%d).\n"
				"This may lead to data truncation.\n",
				bv->bv_len, EB_SECTOR_SIZE);
			ret = -EIO;
		}
		sectors = bv->bv_len / EB_SECTOR_SIZE;
		printk(KERN_DEBUG "eb: Sector Offset: %lld; Buffer: %p; Length: %d sectors\n",
			sector_offset, buffer, sectors);
		if (dir == WRITE) /* Write to the device */
		{
			eeprom_device_write(start_sector + sector_offset, buffer, sectors);
		}
		else /* Read from the device */
		{
			eeprom_device_read(start_sector + sector_offset, buffer, sectors);
		}
		sector_offset += sectors;
	}
	if (sector_offset != sector_cnt)
	{
		printk(KERN_ERR "eb: bio info doesn't match with the request info");
		ret = -EIO;
	}

	return ret;
}
	
/*
 * Represents a block I/O request for us to execute
 */
static void eb_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	/* Gets the current request from the dispatch queue */
	while ((req = blk_fetch_request(q)) != NULL)
	{
#if 0
		/*
		 * This function tells us whether we are looking at a filesystem request
		 * - one that moves block of data
		 */
		if (!blk_fs_request(req))
		{
			printk(KERN_NOTICE "eb: Skip non-fs request\n");
			/* We pass 0 to indicate that we successfully completed the request */
			__blk_end_request_all(req, 0);
			//__blk_end_request(req, 0, blk_rq_bytes(req));
			continue;
		}
#endif
		ret = eb_transfer(req);
		__blk_end_request_all(req, ret);
		//__blk_end_request(req, ret, blk_rq_bytes(req));
	}
}

/* 
 * These are the file operations that performed on the eeprom block device
 */
static struct block_device_operations eb_fops =
{
	.owner = THIS_MODULE,
	.open = eb_open,
	.release = eb_close,
};
	
/* 
 * This is the registration and initialization section of the eeprom block device
 * driver
 */
static int __init eb_init(void)
{
	int ret;

	/* Set up our EEPROM Device */
	if ((ret = eeprom_device_init()) < 0)
	{
		return ret;
	}
	eb_dev.size = ret;

	/* Get Registered */
	eb_major = register_blkdev(eb_major, "eb");
	if (eb_major <= 0)
	{
		printk(KERN_ERR "eb: Unable to get Major Number\n");
		eeprom_device_cleanup();
		return -EBUSY;
	}

	/* Get a request queue (here queue is created) */
	spin_lock_init(&eb_dev.lock);
	eb_dev.eb_queue = blk_init_queue(eb_request, &eb_dev.lock);
	if (eb_dev.eb_queue == NULL)
	{
		printk(KERN_ERR "eb: blk_init_queue failure\n");
		unregister_blkdev(eb_major, "eb");
		eeprom_device_cleanup();
		return -ENOMEM;
	}
	
	/*
	 * Add the gendisk structure
	 * By using this memory allocation is involved, 
	 * the minor number we need to pass bcz the device 
	 * will support this much partitions 
	 */
	eb_dev.eb_disk = alloc_disk(EB_MINOR_CNT);
	if (!eb_dev.eb_disk)
	{
		printk(KERN_ERR "eb: alloc_disk failure\n");
		blk_cleanup_queue(eb_dev.eb_queue);
		unregister_blkdev(eb_major, "eb");
		eeprom_device_cleanup();
		return -ENOMEM;
	}

 	/* Setting the major number */
	eb_dev.eb_disk->major = eb_major;
  	/* Setting the first mior number */
	eb_dev.eb_disk->first_minor = EB_FIRST_MINOR;
 	/* Initializing the device operations */
	eb_dev.eb_disk->fops = &eb_fops;
 	/* Driver-specific own internal data */
	eb_dev.eb_disk->private_data = &eb_dev;
	eb_dev.eb_disk->queue = eb_dev.eb_queue;
	/*
	 * You do not want partition information to show up in 
	 * cat /proc/partitions set this flags
	 */
	//eb_dev.eb_disk->flags = GENHD_FL_SUPPRESS_PARTITION_INFO;
	sprintf(eb_dev.eb_disk->disk_name, "eb");
	/* Setting the capacity of the device in its gendisk structure */
	set_capacity(eb_dev.eb_disk, eb_dev.size);

	/* Adding the disk to the system */
	add_disk(eb_dev.eb_disk);
	/* Now the disk is "live" */
	printk(KERN_INFO "eb: Eeprom Block driver initialised (%d sectors; %d bytes)\n",
		eb_dev.size, eb_dev.size * EB_SECTOR_SIZE);

	return 0;
}
/*
 * This is the unregistration and uninitialization section of the eeprom block
 * device driver
 */
static void __exit eb_cleanup(void)
{
	del_gendisk(eb_dev.eb_disk);
	put_disk(eb_dev.eb_disk);
	blk_cleanup_queue(eb_dev.eb_queue);
	unregister_blkdev(eb_major, "eb");
	eeprom_device_cleanup();
}

module_init(eb_init);
module_exit(eb_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taras Drozdovsky <t.drozdovskiy@gmail.com>");
MODULE_DESCRIPTION("EEPROM Block Driver");
MODULE_ALIAS_BLOCKDEV_MAJOR(eb_major);

/*
 *  eeprom_device.c
 *
 *  The main eeprom device operation functions

 *  Copyright (C) 2017  Taras Drozdovskyi t.drozdovskiy@gmail.com
 */

#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/memory.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/kernel.h>

#include "eeprom_device.h"
#include "partition.h"

/* Start address offset */
#define EEPROM_FS_START_ADDR	0x5000

/* Total eeprom device size = 27 * 512 bytes ~ 13 kB */
#define EB_DEVICE_SIZE 27 /* sectors */

static bool eb_fat16_init = false;
/* Number of fat tables */
static unsigned int eb_fats = 1;
/* Number root entries */
static unsigned int eb_dir_entries = 16;
/* Total number of eeprom sectors to use */
static unsigned int eb_nsectors = EB_DEVICE_SIZE;
/* Type formatting: with/without zeroing of sectors */

module_param(eb_fat16_init, bool, S_IRUGO);
module_param(eb_fats, int, S_IRUGO);
module_param(eb_dir_entries, int, S_IRUGO);
module_param(eb_nsectors, int, S_IRUGO);

/* Memory accessor ralated to i2c memory (See the mach-<arch>-dt.c file) */
extern struct memory_accessor *i2c_mem_acc;

int eeprom_device_init(void)
{
	printk(KERN_INFO "eb: eb_nsectors = %d\n", eb_nsectors);
	if (eb_fat16_init) {
		printk(KERN_INFO "eb: Formatting eeprom memory (min. FAT16 fs ~ %d)\n",
				eb_nsectors * EB_SECTOR_SIZE);

		u8 *dev_data;
		int i;

		/* Check input parameters */
		if (eb_fats != 2)
			eb_fats = 1;

		if (eb_dir_entries > 16)
			eb_dir_entries = 16;

		/* ToDo */
		if (eb_nsectors < 5)
			eb_nsectors = 5;

		dev_data = vmalloc(MBR_SIZE);
		if (dev_data == NULL)
			return -ENOMEM;

		/* Create mbr sector for FAT16 */
		copy_mbr_fat16((fat16_boot_sector *)dev_data, eb_fats, eb_dir_entries,
				eb_nsectors);

		if (i2c_mem_acc->write(i2c_mem_acc, dev_data, EEPROM_FS_START_ADDR,
				MBR_SIZE) != MBR_SIZE)
			printk(KERN_INFO "eb: creation of MBR sector is failed\n");
		if (i2c_mem_acc->write(i2c_mem_acc, dev_data,
				EEPROM_FS_START_ADDR + MBR_SIZE, MBR_SIZE) != MBR_SIZE)
			printk(KERN_INFO "eb: creation of MBR sector is failed\n");

		/* Erasing full sectors */
		memset(dev_data, 0x0, MBR_SIZE);
		for (i = 2; i < eb_nsectors; i++) {
			if (i2c_mem_acc->write(i2c_mem_acc, dev_data,
					EEPROM_FS_START_ADDR + EB_SECTOR_SIZE * i, EB_SECTOR_SIZE)
					!= EB_SECTOR_SIZE) {
				printk(KERN_INFO "eb: sector #%d zeroing is failed\n", i);
				break;
			} else {
				printk(KERN_INFO "eb: formating %d%%\n", i * 100 / eb_nsectors);
			}
		}

		*(unsigned long *)(dev_data) = FAT_EMPTY;
		if(i2c_mem_acc->write(i2c_mem_acc, dev_data,
				EEPROM_FS_START_ADDR + EB_SECTOR_SIZE * 2, FAT_EMPTY_SIZE)
				!= FAT_EMPTY_SIZE)
			printk(KERN_INFO "eb: creation the FAT1 is failed\n");
		if (eb_fats == 2) {
			if(i2c_mem_acc->write(i2c_mem_acc, dev_data,
					EEPROM_FS_START_ADDR + EB_SECTOR_SIZE * 3, FAT_EMPTY_SIZE)
					!= FAT_EMPTY_SIZE)
				printk(KERN_INFO "eb: creation the FAT2 is failed\n");
		}

		vfree(dev_data);

/*
		int j;
		unsigned char mas[1028];
		unsigned char mas1[512];
		for (j = 0; j < eb_nsectors; ++j) {
			if(i2c_mem_acc->read(i2c_mem_acc, mas1, (EEPROM_FS_START_ADDR + (512 * j)), EB_SECTOR_SIZE) != EB_SECTOR_SIZE)
				printk(KERN_INFO "eb: count read number bytes is wrong\n");
			for (i = 0; i < 256; i++)
				hex_byte_pack(mas + (i * 2), mas1[i]);
			mas[i * 2] = '\0';
			printk(KERN_INFO "eb:dev_data[%d][%d]:\n%s", j, 512, mas);
			for (i = 0; i < 256; i++)
				hex_byte_pack(mas + (i * 2), mas1[i+256]);
			mas[i * 2] = '\0';
			printk(KERN_INFO "%s\n", mas);
		}
*/
	}
	printk(KERN_INFO "eb: eeprom_device_init is completed\n");
	return eb_nsectors;
}

void eeprom_device_write(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	if (i2c_mem_acc->write(i2c_mem_acc, buffer,
			(EEPROM_FS_START_ADDR + (sector_off * EB_SECTOR_SIZE)),
			sectors * EB_SECTOR_SIZE) != sectors * EB_SECTOR_SIZE);
}
void eeprom_device_read(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	if (i2c_mem_acc->read(i2c_mem_acc, buffer,
			(EEPROM_FS_START_ADDR + (sector_off * EB_SECTOR_SIZE)),
			sectors * EB_SECTOR_SIZE) == sectors * EB_SECTOR_SIZE);
}

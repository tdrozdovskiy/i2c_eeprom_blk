#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/memory.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>

#include "eeprom_device.h"
#include "partition.h"

#define EB_DEVICE_SIZE 27 /* 26 or 27 */ /* sectors */
/* So, total device size = 32 * 512 bytes = 16 KiB */

/* Array where the disk stores its data */
static u8 *dev_data;

extern struct memory_accessor* get_i2c_memory_accessor(void);

#define EEPROM_FS_BASE_ADDR	0x5000

static struct memory_accessor *mem_acc;

int eeprom_device_init(void)
{
	mem_acc = get_i2c_memory_accessor();

	dev_data = vmalloc(EB_DEVICE_SIZE * EB_SECTOR_SIZE);
	if (dev_data == NULL)
		return -ENOMEM;

	/* Setup its partition table */
	copy_mbr_n_br(dev_data);

	if(mem_acc->write(mem_acc, dev_data, EEPROM_FS_BASE_ADDR, EB_SECTOR_SIZE) == EB_SECTOR_SIZE);

	return EB_DEVICE_SIZE;
}

void eeprom_device_cleanup(void)
{
	vfree(dev_data);
}

void eeprom_device_write(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	if(mem_acc->write(mem_acc, buffer, (EEPROM_FS_BASE_ADDR + (sector_off * EB_SECTOR_SIZE)),
			sectors * EB_SECTOR_SIZE) == sectors * EB_SECTOR_SIZE);
}
void eeprom_device_read(sector_t sector_off, u8 *buffer, unsigned int sectors)
{
	if(mem_acc->read(mem_acc, buffer, (EEPROM_FS_BASE_ADDR + (sector_off * EB_SECTOR_SIZE)),
			sectors * EB_SECTOR_SIZE) == sectors * EB_SECTOR_SIZE);
}

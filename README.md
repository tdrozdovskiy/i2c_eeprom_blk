# AT24 EEPROM block device Linux driver
Sometimes there is a need to store data in a eeprom memory. But to work with it need to develope a special program. This driver allows to create a standard block device that can be used as disk for storing data.

## Changes of the Linux kernel

To work with **AT24cxx** eeprom memory, need to configure the linux kernel by setting the option:
`CONFIG_EEPROM_AT24=y`
After this, modify the `arch/arm/mach-<your_arch>/mach-<your_arch>-dt.c` file by added the next code:
```c
#ifdef CONFIG_EEPROM_AT24
#include <linux/i2c/at24.h>
#endif
...
#ifdef CONFIG_EEPROM_AT24
struct memory_accessor *i2c_mem_acc = NULL;
EXPORT_SYMBOL(i2c_mem_acc);

static void get_mem_acc(struct memory_accessor *mem_acc, void *context)
{
	if(mem_acc == NULL)
		printk(KERN_INFO "mem_acc is not initialized for AT24's device\n");
	i2c_mem_acc = mem_acc;
}

static struct at24_platform_data eeprom_info = {
		.byte_len       = (256*1024) / 8,
		.page_size      = 64,
		.flags          = AT24_FLAG_ADDR16,
		.setup          = get_mem_acc,
		.context        = NULL,
};
#endif /* CONFIG_EEPROM_AT24 */

/*****************************************************************************
  * I2C Devices
  ****************************************************************************/
static struct i2c_board_info i2c_devs0[] __initdata = {
...
#ifdef CONFIG_EEPROM_AT24
    {
        /* 0x56 - registered address for STM i2c eeprom */
        I2C_BOARD_INFO("24c256",0x56),
        .platform_data = &eeprom_info,
    },
#endif
...
 };

```
## Build the at24cxx eeprom block device driver
To build eeprom driver run the command:
```sh
$ make
```
after this `eepromblk.ko` will be ready to install.

The driver can be installed with the following parameters:
* `eb_fat16_init`
* `eb_fats`
* `eb_dir_entries`
* `eb_nsectors`

`eb_fat16_init` - type is `bool`. **True** (defined) means that the eeprom memory will be formatted as a min FAT16 file system:

| Number sector | Description  |
| ------------- | ------------ |
| 0             | Disk geometry|
| 1             | MBR          |
| 2             | FAT1 table   |
| 3             | Root entries |
| 4 - n         | Data         |

The following parameters allow to more flexibly configure the minimal FAT16 file system.

`eb_fats` - type is `int`. Possible value are: **1 (default) or 2**. Number FAT tables for FAT16 file system.
`eb_dir_entries` - type is `int`. Number of inodes that can be created in the root directory. **16** by default (occupies one sector). The size of each dir_entry is 32 bytes.
`eb_nsectors` - type is `int`. Number of sectors allocated to the file system. **27** by default.

Example as installing the eeprom block device drivers find the below:
##### 1. Creation minimal FAT16 file system
```sh
# insmod eepromblk.ko eb_fat16_init
```

##### 2. Creation minimal FAT16 file system with FAT1, FAT2 tables and number of dir_entries = 10
```sh
# insmod eepromblk.ko eb_fat16_init eb_fats=2 eb_direntries=10
```

##### 3. Use an existing file system without any changes
```sh
# insmod eepromblk.ko
```

> The use of the first and second variant results in the deletion of all data.

After the eepromblk driver has been successfully installed, the system will register the eeprom block device `/dev/eb1`.
When using `AT24c512` or `AT24c1024` memory, formatting can be done using standard programs like as `mkdosfs`.
```sh
# mkdosfs -F 16 -f 1 -s 1 -S 512 -r 16 /dev/eb1
```
Otherwise, need to use option 1 or 2 once to create the file system.
```sh
# insmod eepromblk.ko eb_fat16_init
```

To mounting of the file system, execute the  next command:
```sh
mount -t vfat /dev/eb1 /<your_mount_point_path/
```
> The driver has been tested on the **linux kernel v3.11** and the **AT24c256** eeprom memory.

To use only part of the memory, specify the start address in the `eeprom_device.c` file.
```c
/* Start address offset */
#define EEPROM_FS_START_ADDR	0x0
```

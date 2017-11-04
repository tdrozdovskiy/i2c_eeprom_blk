/*
 * partition.h - partitions definition
 *
 * Copyright (C) 2017  Taras Drozdovskyi t.drozdovskiy@gmail.com
 */

#ifndef _PARTITION_H_
#define _PARTITION_H_

#include <linux/types.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

#define SECTOR_SIZE 512
#define MBR_SIZE SECTOR_SIZE
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16
#define PARTITION_TABLE_SIZE 64
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55
#define BR_SIZE SECTOR_SIZE
#define BR_SIGNATURE_OFFSET 510
#define BR_SIGNATURE_SIZE 2
#define BR_SIGNATURE 0xAA55

#define FAT_EMPTY_SIZE 4
#define FAT_EMPTY      0xFFFFFFF8

typedef struct {
	unsigned char boot_type; // 0x00 - Inactive; 0x80 - Active (Bootable)
	unsigned char start_head;
	unsigned char start_sec :6;
	unsigned char start_cyl_hi :2;
	unsigned char start_cyl;
	unsigned char part_type;
	unsigned char end_head;
	unsigned char end_sec :6;
	unsigned char end_cyl_hi :2;
	unsigned char end_cyl;
	unsigned long abs_start_sec;
	unsigned long sec_in_part;
} part_entry;

#pragma pack(push, 1)
typedef struct {
	unsigned char  jump_instr[3]; /* Boot strap short or near jump */
	unsigned char  os_fs_id[8];   /* test id of OS or FS */
	unsigned short sector_size;   /* bytes per logical sector */
	unsigned char  sec_per_clus;  /* sectors/cluster */
	unsigned short reserved;      /* reserved sectors */
	unsigned char  fats;          /* number of FATs */
	unsigned short dir_entries;   /* root directory entries */
	unsigned short sectors;       /* number of sectors */
	unsigned char  media;         /* media code */
	unsigned short fat_length;    /* sectors/FAT */
	unsigned short secs_track;    /* sectors per track */
	unsigned short heads;         /* number of heads */
	unsigned int   hidden;        /* hidden sectors (unused) */
	unsigned int   total_sect;    /* number of sectors (if sectors == 0) */
	unsigned char  phys_disk_num; /* physical disk number */
	unsigned char  cur_hed_res;   /* current head (reserved) */
	unsigned char  signature;     /* signature */
	unsigned int   vol_ser_num;   /* volume serial number */
	unsigned char  volume_label[11]; /* volume label */
	unsigned char  system_id[8];  /* system id */
	unsigned char  code[378];     /* code of bootloader */
	unsigned int   mbr_signature;
	unsigned short reserved1;
	part_entry     part_table[4];
	unsigned char  boot_sign[2];  /* boot signature 55AAh - the end of boot sector */
} fat16_boot_sector;
#pragma pack(pop)

void copy_mbr_fat16(fat16_boot_sector *mbr, int fats, int dir_entries,
		int nsectors);
#endif /* _PARTITION_H_ */

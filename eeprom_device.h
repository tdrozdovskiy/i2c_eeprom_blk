/*
 * eeprom_device.h - header for eeprom AT24cxx device
 *
 * Copyright (C) 2017  Taras Drozdovskyi t.drozdovskiy@gmail.com
 */

#ifndef _EEPROMDEVICE_H_
#define _EEPROMDEVICE_H_

#define EB_SECTOR_SIZE 512

extern int eeprom_device_init(void);
extern void eeprom_device_write(sector_t sector_off, u8 *buffer,
		unsigned int sectors);
extern void eeprom_device_read(sector_t sector_off, u8 *buffer,
		unsigned int sectors);
#endif /* _EEPROMDEVICE_H_ */

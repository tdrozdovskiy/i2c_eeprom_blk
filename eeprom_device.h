#ifndef EEPROMDEVICE_H
#define EEPROMDEVICE_H

#define EB_SECTOR_SIZE 512

extern int eeprom_device_init(void);
extern void eeprom_device_cleanup(void);
extern void eeprom_device_write(sector_t sector_off, u8 *buffer, unsigned int sectors);
extern void eeprom_device_read(sector_t sector_off, u8 *buffer, unsigned int sectors);
#endif

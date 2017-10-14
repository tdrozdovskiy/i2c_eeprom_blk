# AT24 EEPROM block device Linux driver

`CONFIG_EEPROM_AT24=y`

To apply a patch (i2c_eeprom_blk_patch) to Linux kernel by running the commands:
```sh
$ cd <P-KNOX>/kernel
$ patch -p1 < i2c_eeprom_blk_patch
```

Build Linux kernel by running the command:
```sh
$ ./build.sh
```

Extract and build "i2c_eeprom_blk"
```sh
$ tar xzf i2c_eeprom_blk.tar.gz
$ cd i2c_eeprom_blk
$ make
```

Copy "eeprom_fat16_fs" and "eepromblk.ko" to /root/temp/ on Transformer board
Insmod "eepromblk.ko" driver
```sh
# insmod /root/temp/eepromblk.ko
```

Copy FAT16 header file system into the EEPROM
```sh
# dd if=/root/temp/eeprom_fat16_fs of=/dev/eb1 bs=64 count=150
```

```sh
#!/bin/bash
sudo dd if=/dev/zero of=my_fs bs=64 count=1024
sudo losetup /dev/loop0 my_fs
sudo mkdosfs -F 16 -f 1 -s 1 -S 512 -r 16 /dev/loop0
sudo mount -t vfat /dev/loop0 /mnt/fs_mount_location/
sudo cp -a <your files> /mnt/fs_mount_location/
sudo sync
```

```sh
$ echo hello world > a
$ xxd -i a
```

#!/bin/bash

# takes an executable file "kernel.elf" in the current directory
# and creates a 50MB bootable disk image "disk.img" with jackos bootloader and the kernel
# uses sudo to create a loop device and mount it

set -ex
dd if=/dev/zero of=disk.img bs=1M count=50
dd if=src/bootsect/bootsect.bin of=disk.img bs=512 seek=0 conv=notrunc
dd if=src/stage2/stage2.bin of=disk.img bs=512 seek=2 conv=notrunc
sudo losetup -D
sudo losetup -fP disk.img
sudo mkfs.vfat -F 16 /dev/loop0p1
sudo mount -m /dev/loop0p1 mnt
sudo cp -r kernel.elf mnt/
sudo umount mnt
sudo losetup -D
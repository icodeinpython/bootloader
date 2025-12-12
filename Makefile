


disk.img: bootsect stage2 test_kernel
	dd if=/dev/zero of=disk.img bs=512 count=102400
	dd if=src/bootsect/bootsect.bin of=disk.img bs=512 seek=0 conv=notrunc
	dd if=src/stage2/stage2.bin of=disk.img bs=512 seek=2 conv=notrunc
	sudo losetup -D
	sudo losetup -fP disk.img
	sudo mkfs.vfat -F 16 /dev/loop0p1
	sudo mount /dev/loop0p1 mnt
	sudo cp -r src/test_kernel/kernel.elf mnt/
	sudo umount mnt
	sudo losetup -D

test_kernel:
	$(MAKE) -C src/test_kernel

bootsect:
	$(MAKE) -C src/bootsect

qemu: disk.img
	qemu-system-x86_64 -hda disk.img

bochs: disk.img
	bochs -f bochsrc.txt -dbg_gui

stage2:
	$(MAKE) -C src/stage2

clean:
	$(MAKE) -C src/stage2 clean
	$(MAKE) -C src/bootsect clean
	rm -f disk.img
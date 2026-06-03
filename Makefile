
default: disk.img

disk.img: bootsect stage2 kernel.elf
	bash makeDisk.sh
kernel.elf:
	$(MAKE) -C src/test_kernel
	cp src/test_kernel/kernel.elf kernel.elf

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
CC = i686-elf-gcc
CFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector -Wall -Wextra -pedantic -Werror -I include -Oz -Wno-unused
	
LDFLAGS = -m32 -ffreestanding -nostdlib -fno-stack-protector -Wl,--build-id=none -Wl,--gc-sections -T linker.ld

C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.S)
OBJ = $(C_SRC:.c=.c.o) $(ASM_SRC:.S=.S.o)

stage2.bin: $(OBJ) lm_init.bin
	$(CC) $(LDFLAGS) -o $@ $(OBJ)
	truncate -s 30K stage2.bin
	cat lm_init.bin >> $@

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.S.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	find -name '*.o' -delete
	find -name '*.bin' -delete

lm_init.bin: lm_init.o linker64.ld
	x86_64-elf-ld -o $@ -T linker64.ld $<
	truncate -s 1K $@

lm_init.o: lm_init.asm
	nasm -f elf64 -o $@ $<
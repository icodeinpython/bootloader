CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -nostdlib -fno-stack-protector -Wall -Wextra -pedantic -Werror -I include -O2 -Wno-unused -I.
	
LDFLAGS = -m64 -ffreestanding -nostdlib -fno-stack-protector -Wl,--build-id=none -Wl,--gc-sections -T linker.ld

C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.S)
OBJ = $(C_SRC:.c=.c.o) $(ASM_SRC:.S=.S.o)

kernel.elf: $(OBJ) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.S.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	find -name '*.o' -delete
	find -name '*.bin' -delete
	rm -f kernel.elf
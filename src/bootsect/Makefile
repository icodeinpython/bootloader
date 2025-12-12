ASM_SRCS := $(wildcard *.asm)


bootsect.bin: $(ASM_SRCS)
	nasm -f bin -o $@ bootsect.asm

clean:
	rm -f *.bin
.global kernel_jmp
// void kernel_jmp(void* addr);
#define MAGIC 0xB19B00B5
.extern boot_info
kernel_jmp:
    xchg %bx, %bx
    movl 4(%esp), %ecx
    movl $MAGIC, %eax
    movl boot_info, %ebx
    jmp *%ecx
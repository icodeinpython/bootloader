.global _start
_start:
    mov $0x7c00, %esp
    push %ebx
    call cmain
1:  jmp 1b
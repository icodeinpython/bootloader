.global _start
_start:
    mov $0x7c00, %rsp
    mov %eax, %edi
    mov %rbx, %rsi
    mov %rcx, %rdx

    call cmain
1:  jmp 1b
.section .text.entry
.global entry
.extern cmain
.extern mmap_count
entry:
    movl %ebx, mmap_count
    mov $0x7c00, %esp
    call cmain
1:  jmp 1b
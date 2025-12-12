jmpPmode:
    cli
    call enableA20
    lgdt [GDTR]
    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp 0x08:pmode




GDT:
.zero: dq 0
.code: dq 0x00CF9A000000FFFF ; base, limit, type, dpl, present, avl
.data: dq 0x00CF92000000FFFF ; base, limit, type, dpl, present, avl

GDTR:
.size: dw GDTR - GDT - 1
.offset: dd GDT

enableA20:
    in al, 0x92
    test al, 2
    jnz .done
    or al, 2
    and al, 0xFE
    out 0x92, al
.done:
    ret


[bits 32]
pmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7c00

    mov ebx, [mmap_count]
    
    jmp 0x7c00

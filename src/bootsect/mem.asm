mmap_addr: dd 0x2000
mmap_count: dd 0

get_mmap:
    xchg bx, bx
    xor cx, cx
    xor di, di
    mov eax, dword [mmap_addr]
    shr eax, 4
    mov es, ax ; mmap = es:di = 0x0200:0x0000
    xor ebx, ebx

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24
    int 0x15
    jc .done

    cmp eax, 0x534D4150
    jne .done

    add di, 24
    inc dword [mmap_count]

    test ebx, ebx
    jnz .next_entry
.done:
    xor di, di
    
    movzx eax, cx
    mov [mmap_count], eax
    ret
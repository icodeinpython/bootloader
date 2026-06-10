mmap_addr: dd 0x2010
mmap_count equ 0x2000

get_mmap:
    ; clear counter
    mov dword [mmap_count], 0

    ; set up destination segment
    mov eax, dword [mmap_addr]
    shr eax, 4 ; divide by 16 to get segment
    mov es, ax
    xor di, di ; es:di = 0x0201:0x0000

    xor ebx, ebx ; ebx = 0, used as offset into the mmap buffer

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150 ; 'SMAP'
    mov ecx, 24 ; size of the buffer for the entry
    int 0x15

    jc .done

    cmp eax, 0x534D4150
    jne .done

    ; increment entry count
    inc dword [mmap_count]

    ; move es forward by 32 bytes for the next entry so di stays 0. This prevents di from wrapping at 64 KiB
    mov ax, es
    add ax, 2
    mov es, ax

    ; check if ebx was reset to 0 by bios, which indicates there are no more entries
    test ebx, ebx
    jnz .next_entry

.done:
    ret
org 0x600

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    .copyLower:
        mov cx, 0x0100
        mov si, 0x7C00
        mov di, 0x600
        rep movsw
    jmp 0:LowStart

LowStart:
    mov [bootDrive], dl

    ; disable cursor
    mov ah, 0x01
    mov ch, 0x3f
    int 0x10

    mov si, welcome
    call print_string
    call checkint13ext

; load second stage
    mov bx, 0x800
    mov eax, 1
    mov cx, 1
    call readSectors


    push word 0x00
    pop es
    mov bx, 0x7c00
    mov eax, 2
    mov cx, 62
    call readSectors
    call video_init
    call get_mmap
    call jmpPmode
    jmp $


%include "disk.asm"
%include "print.asm"
%include "pmode.asm"
[bits 16]

welcome: db "Welcome to JackOS", 0x0d, 0x0a, 0

diskTimeStamp: times 8 db 0
bootDrive: db 0
PToff: dw 0

times (0x1b8 - ($-$$)) nop

UID times 6  db 0             ; Unique Disk ID
; PT1
PT1:
.driveAttr: db 0x80
.chsStart: times 3 db 0
.partitionType: db 0x0c       ; FAT32
.endCHS: times 3 db 0
.startLBA: dd 64              ; LBA of first sector of partition
.sectors: dd 40960               ; Sectors in partition
PT2 times 16 db 0             ; Second Partition Entry
PT3 times 16 db 0             ; Third Partition Entry
PT4 times 16 db 0             ; Fourth Partition Entry


times 510-($-$$) db 0
dw 0xaa55
%include "video.asm"
%include "mem.asm"
times (1024-($-$$)) nop
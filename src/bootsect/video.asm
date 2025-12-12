[BITS 16]

%define WIDTH  1024
%define HEIGHT 768
%define BPP    32


; buffers: 0xA00 - 0xC00 = VBE_INFO_BLOCK
; 0xC00 - 0xE00 = MODE_INFO_BLOCK

; Buffer offset in same segment (just after code, 512 bytes max)
VBE_INFO_BLOCK equ 0x0A00
MODE_INFO_BLOCK equ 0x0C00
SCREEN equ 0x1000

video_error:
    mov si, .msg
    call print_string
    jmp $
.msg: db "Error setting video resolution", 0xA, 0xD, 0


video_init:
    push es ; some bioses trash es
    mov dword [VBE_INFO_BLOCK], "VBE2"
    mov ax, 0x4F00
    mov di, VBE_INFO_BLOCK
    int 0x10
    pop es

    cmp ax, 0x004F
    jne video_error

    cmp dword [VBE_INFO_BLOCK], "VESA"
    jne video_error

    cmp dword [VBE_INFO_BLOCK+4], 0x200
    jl video_error

    mov ax, WIDTH
    mov bx, HEIGHT
    mov cl, 32
    call vbe_set_mode
    jc video_error
    ret

; vbe_set_mode:
; Sets a VBE mode
; In\	AX = Width
; In\	BX = Height
; In\	CL = Bpp
; Out\	FLAGS.CF = 0 on success, 1 on error

vbe_set_mode:
    mov [.width], ax
    mov [.height], bx
    mov [.bpp], cl

    push es
    mov dword [VBE_INFO_BLOCK], "VBE2"
    mov ax, 0x4F00
    mov di, VBE_INFO_BLOCK
    int 0x10
    pop es

    cmp ax, 0x004F
    jne video_error

    mov ax, word [VBE_INFO_BLOCK+14]
    mov [.offset], ax
    mov ax, word [VBE_INFO_BLOCK+16]
    mov [.segment], ax

    mov ax, word [.segment]
    mov fs, ax
    mov si, word [.offset]

.find_mode:
    mov dx, [fs:si]
    add si, 2
    mov [.offset], si
    mov [.mode], dx
    xor ax, ax
    mov fs, ax

    cmp word [.mode], 0xFFFF
    je video_error

    push es
    mov ax, 0x4F01
    mov cx, [.mode]
    mov di, MODE_INFO_BLOCK
    int 0x10
    pop es

    cmp ax, 0x004f
    jne video_error


    mov ax, [.width]
    cmp ax, word [MODE_INFO_BLOCK+18]
    jne .next_mode
    
    mov ax, [.height]
    cmp ax, word [MODE_INFO_BLOCK+20]
    jne .next_mode

    movzx ax, byte [MODE_INFO_BLOCK+25]
    cmp al, byte [.bpp]
    jne .next_mode

    test word [MODE_INFO_BLOCK], 0x81
    jz .next_mode

    push es
    mov ax, 0x4F02
    mov bx, [.mode]
    or bx, 0x4000
    xor di, di
    int 0x10
    pop es

    cmp ax, 0x004F
    jne video_error

    movzx eax, word [.width] ; width
    mov [SCREEN], eax
    movzx eax, word [.height] ; height
    mov [SCREEN+4], eax
    movzx eax, byte [.bpp] ; bpp
    mov [SCREEN+8], eax
    add eax, 7
    shr eax, 3
    mov [SCREEN+12], eax ; bytes per pixel

    movzx eax, word [MODE_INFO_BLOCK+16] ; pitch
    mov [SCREEN+16], eax
    
    mov eax, dword [MODE_INFO_BLOCK+40] ; framebuffer
    mov [SCREEN+28], eax
    clc
    ret

.next_mode:
    mov ax, [.segment]
    mov fs, ax
    mov si, [.offset]
    jmp .find_mode


.width				dw 0
.height				dw 0
.bpp				db 0
.segment			dw 0
.offset				dw 0
.mode				dw 0



; SCREEN layout:
; offset    | length    | item
; 0         | 4         | width
; 4         | 4         | height
; 8         | 4         | bpp
; 12        | 4         | bytes per pixel
; 16        | 4         | bytes per line (pitch)
; 20        | 4         | screen size
; 24        | 4         | screen_size_dqwords for sse
; 28        | 4         | framebuffer

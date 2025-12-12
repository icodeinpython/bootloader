DAP:
.size           db 0x10         ; size of DAP 0
.rsvd           db 0x00         ; reserved  1
.num_sectors    dw 62       ; number of sectors 2
.buf_offset     dw 0x7c00       ; offset of buffer 4
.buf_seg        dw 0x0000       ; segment of buffer 6
.lba_low        dd 0x00000000   ; LBA low 8
.lba_high       dd 0x00000000   ; LBA high 12


checkint13ext:
    mov ah, 0x41
    mov bx, 0x55aa
    clc
    mov dl, [bootDrive]
    int 0x13
    jc error
    ret
error:
    mov si, errorMsg
    call print_string
    jmp $
errorMsg: db "Error reading disk", 0


; eax = LBA
; read to [es:bx]
; cs: number of sectors to read
readSectors:
    mov word [DAP.num_sectors], cx
    mov dword [DAP.lba_low], eax
    mov dword [DAP.lba_high], 0
    mov word [DAP.buf_offset], bx
    mov word [DAP.buf_seg], es
    mov dl, [bootDrive]
    mov ah, 0x42
    mov si, DAP
    int 0x13
    jc error
    ret

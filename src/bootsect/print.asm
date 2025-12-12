; ds:si = string
print_string:
    pusha
.next_char:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0e
    xor bh, bh
    mov bl, 0x07
    int 0x10
    jmp .next_char
.done:
    popa
    ret
main:
    mov  x1, 97
beg:
    cmp  x1, 123
    jeq  enb
    mov  x0, x1
    call putchar
    add  x1, x1, 1
    jmp  beg
enb:
    mov  x0, 10
    call putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 10
    call putchar
    mov  x0, 0xFF
    ret
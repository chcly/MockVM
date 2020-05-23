main:
    mov  x1, 122
beg:
    cmp  x1, 97
    jlt  enb
    mov  x0, x1
    call putchar
    sub  x1, x1, 1
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

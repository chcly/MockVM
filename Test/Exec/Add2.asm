main:
    mov  x1, 97
beg:
    cmp  x1, 123
    beq  enb
    mov  x0, x1
    bl   putchar
    add  x1, x1, 1
    b    beg
enb:
    mov  x0, 10
    bl  putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 10
    bl   putchar
    mov  x0, 0xFF
    ret
main:
    mov  x1, 'a'
beg:
    cmp  x1, 'z'
    bgt  enb
    mov  x0, x1
    bl   putchar
    add  x1, 1
    b    beg
enb:
    mov  x0, '\n'
    bl  putchar
    mov  x0, 0
    ret

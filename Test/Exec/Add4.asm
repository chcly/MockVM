main:
    mov  x1, ' '
beg:
    cmp  x1, '~'
    bgt  enb
    mov  x0, x1
    bl putchar
    add  x1, 1
    b  beg
enb:
    mov  x1, 224

beg1:
    cmp  x1, 253
    bgt  end1
    mov  x0, x1
    bl putchar
    add  x1, 1
    b  beg1
end1:
    mov  x0, '\n'
    bl putchar
    mov  x0, 50
    bl putchar
    mov  x0, 252
    bl putchar
    mov  x0, 50
    bl putchar
    mov  x0, 227
    bl putchar
    mov  x0, 50
    bl putchar
    mov  x0, 248
    bl putchar
    mov  x0, '\n'
    bl putchar
    mov  x0, 0
    ret

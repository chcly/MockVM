main:
    mov  x1, ' '
beg:
    cmp  x1, '~'
    jgt  enb
    mov  x0, x1
    call putchar
    add  x1, 1
    jmp  beg
enb:
    mov  x1, 224

beg1:
    cmp  x1, 253
    jgt  end1
    mov  x0, x1
    call putchar
    add  x1, 1
    jmp  beg1
end1:
    mov  x0, '\n'
    call putchar
    mov  x0, 50
    call putchar
    mov  x0, 252
    call putchar
    mov  x0, 50
    call putchar
    mov  x0, 227
    call putchar
    mov  x0, 50
    call putchar
    mov  x0, 248
    call putchar
    mov  x0, '\n'
    call putchar
    mov  x0, 0
    ret
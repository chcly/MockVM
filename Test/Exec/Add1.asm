main:
    mov  x1, 'a'
beg:
    cmp  x1, 'z'
    jgt  enb
    mov  x0, x1
    call putchar
    add  x1, 1
    jmp  beg
enb:
    mov  x0, '\n'
    call putchar
    mov  x0, 0
    ret
fail:
    mov  x0, '\n'
    call putchar
    mov  x0, 0xFF
    ret

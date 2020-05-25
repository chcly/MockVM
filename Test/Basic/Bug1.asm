main:
    call  change_cur_inst
    mov  x0, 'P'
    call putchar
    ret
weee:
    mov  x0, 'H'
    call putchar
    mov  x0, 'e'
    call putchar
    mov  x0, 'l'
    call putchar
    mov  x0, 'l'
    call putchar
    mov  x0, '0'
    call putchar
    mov  x0, '\n'
    call putchar
    mov  x0, 0
    ret

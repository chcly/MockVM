main:
    bl change_cur_inst
    mov  x0, 'P'
    bl putchar
    mov  x0, 0
    ret
weee:
    mov  x0, 'H'
    bl putchar
    mov  x0, 'e'
    bl putchar
    mov  x0, 'l'
    bl putchar
    mov  x0, 'l'
    bl putchar
    mov  x0, '0'
    bl putchar
    mov  x0, '\n'
    bl putchar
    mov  x0, 0
    ret
    
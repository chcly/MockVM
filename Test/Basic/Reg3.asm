; testing an early exit
main:
    mov w0, 'B'
    bl putchar
    mov w0, '4'
    bl putchar
    mov pc, -1
    mov w0, 'E'
    bl putchar
    mov w0, 'C'
    bl putchar
done:
    mov w0, 'P'
    bl putchar
    mov w0, '\n'
    bl putchar
    prgi
    mov w0, 0
    ret

; testing an early exit
main:
    mov w0, 'B'
    call putchar
    mov w0, '4'
    call putchar
    mov pc, -1
    mov w0, 'E'
    call putchar
    mov w0, 'C'
    call putchar
done:
    mov w0, 'P'
    call putchar
    mov w0, '\n'
    call putchar
    prgi
    mov w0, 0
    ret

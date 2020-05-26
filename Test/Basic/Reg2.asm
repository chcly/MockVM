; Test using the program counter to 
; jump. The correct output should be 'B4P'
; and a dump of the contents of the registers.
; Instructions are [ 0 - (N-1) ] if this boundary 
; is exceeded the program will terminate.
main:
    mov w0, 'B'
    call putchar
    mov w0, '4'
    call putchar
    mov pc, 9
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

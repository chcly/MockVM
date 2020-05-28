factorial:
    mov   x1, 1
    mov   x2, x0
facst:
    cmp   x1,x2
    bge   facen
    mul   x0,x1
    inc   x1
    b     facst
facen:
    ret

main:
    mov  x0, 12
    bl   factorial
    prg  x0
    mov  x0, 0
    ret

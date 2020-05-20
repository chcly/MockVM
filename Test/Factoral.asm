// ----------------------------------------------------------------------------
// int factorial(int a) 
//  [ data+8  ] = a
// ----------------------------------------------------------------------------
main:
    mov x0, 16
    mov x1, 1
    call factorial
    prg x0
    mov x0, 0
    ret



factorial:
    mov x2, x0
facst:
    cmp   x0,x1
    jeq   facen
    mul   x2,x1
    inc   x1
    jmp   facst
facen:
    mov x0, x2
    ret
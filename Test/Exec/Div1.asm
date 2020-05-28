main:
    mov x1, 0xF0 
    mov x7, 2
top:
    cmp x1, 0
    beq pass
    div x2, x1, x7
    mul x3, x2, x7
    sub x0, x1, x3
    mov x1, x2
    cmp x0, 1
    beq L1
    add x0, 48
    bl putchar
    b top
L1:
    add x0, 48
    bl putchar
    b top
pass:
    mov  x0, 10
    bl putchar
    mov  x0, 0
    ret

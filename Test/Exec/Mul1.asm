main:
    mov  x3, 1

top:    
    cmp  x3, 10
    bgt pass
    mul x4, x3, 1
    prg x4
    cmp x4, x3
    beq L1
    b fail
L1:
    add x5, x3, x3
    mul x4, x3, 2
    prg x4
    cmp x4, x5
    beq L2
    b fail

L2:
    add x5, x5, x3
    mul x4, x3, 3
    prg x4
    cmp x4, x5
    beq L3
    b fail
L3:
    add x5, x5, x3
    mul x4, x3, 4
    prg x4
    cmp x4, x5
    beq L4
    b fail
L4:
    add x5, x5, x3
    mul x4, x3, 5
    prg x4
    cmp x4, x5
    beq L5
    b fail
L5:
    add x5, x5, x3
    mul x4, x3, 6
    prg x4
    cmp x4, x5
    beq L6
    b fail
L6:
    add x5, x5, x3
    mul x4, x3, 7
    prg x4
    cmp x4, x5
    beq L7
    b fail
L7:
    add x5, x5, x3
    mul x4, x3, 8
    prg x4
    cmp x4, x5
    beq L8
    b fail
L8:
    add x5, x5, x3
    mul x4, x3, 9
    prg x4
    cmp x4, x5
    beq L9
    b fail
L9:
    add x5, x5, x3
    mul x4, x3, 10
    prg x4
    cmp x4, x5
    beq L10
    b fail
L10:
    mov  x0, 10
    bl putchar
    mov  x0, 0x50
    bl putchar
    mov  x0, 10
    bl putchar
    inc x3
    b top
pass:
    mov  x0, 10
    bl putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 0x46
    bl putchar
    mov  x0, 10
    bl putchar
    mov  x0, 0xFF
    ret
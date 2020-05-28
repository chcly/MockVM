main:
    mov  x1, -10
    cmp x1, -10
    prg x1
    beq L1
    b fail
L1:
    add x1, 1
    prg x1
    cmp x1, -9
    beq L2
    b fail
L2:
    sub x1, 2
    prg x1
    cmp x1, -11
    beq L3
    b fail
L3:
    sub x1, -2
    prg x1
    cmp x1, -9
    beq L4
    b fail
L4:
    sub x1, -9
    prg x1
    cmp x1, 0
    beq L5
    b fail
L5:
    mov x1, 0
    sub x1, 0xFFFFFFFFFFFFFFFF
    prg x1
    cmp x1, 1
    beq L6
    b fail
L6:
    mov x1, 0
    sub x1, 0xFFFFFFFFFFFFFFFE
    prg x1
    cmp x1, 2
    beq L7
    b fail
L7:
    mov x1, -1
    sub x1, 0xFFFFFFFFFFFFFFFF
    prg x1
    cmp x1, 0
    beq L8
    b fail
L8:
    mov x1, -1
    sub x1, 0xFFFFFFFFFFFFFFFE
    prg x1
    cmp x1, 1
    beq pass
    b fail
pass:
    mov  x0, 0x50
    bl putchar
    mov  x0, 0x0A
    bl putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 0x46
    bl putchar
    mov  x0, 0x0A
    bl putchar
    mov  x0, 0
    ret
main:
    mov  x3, 1
top:    
    cmp  x3, 10
    jgt pass
    mul x4, x3, 1
    prg x4
    cmp x4, x3
    jeq L1
    jmp fail
L1:
    add x5, x3, x3
    mul x4, x3, 2
    prg x4
    cmp x4, x5
    jeq L2
    jmp fail

L2:
    add x5, x5, x3
    mul x4, x3, 3
    prg x4
    cmp x4, x5
    jeq L3
    jmp fail
L3:
    add x5, x5, x3
    mul x4, x3, 4
    prg x4
    cmp x4, x5
    jeq L4
    jmp fail
L4:
    add x5, x5, x3
    mul x4, x3, 5
    prg x4
    cmp x4, x5
    jeq L5
    jmp fail
L5:
    add x5, x5, x3
    mul x4, x3, 6
    prg x4
    cmp x4, x5
    jeq L6
    jmp fail
L6:
    add x5, x5, x3
    mul x4, x3, 7
    prg x4
    cmp x4, x5
    jeq L7
    jmp fail
L7:
    add x5, x5, x3
    mul x4, x3, 8
    prg x4
    cmp x4, x5
    jeq L8
    jmp fail
L8:
    add x5, x5, x3
    mul x4, x3, 9
    prg x4
    cmp x4, x5
    jeq L9
    jmp fail
L9:
    add x5, x5, x3
    mul x4, x3, 10
    prg x4
    cmp x4, x5
    jeq L10
    jmp fail
L10:
    mov  x0, 10
    call putchar
    mov  x0, 0x50
    call putchar
    mov  x0, 10
    call putchar
    inc x3
    jmp top
pass:
    mov  x0, 10
    call putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 0x46
    call putchar
    mov  x0, 10
    call putchar
    mov  x0, 0xFF
    ret
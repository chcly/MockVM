main:
    mov  x1, -10
    cmp x1, -10
    prg x1
    jeq L1
    jmp fail
L1:
    add x1, 1
    prg x1
    cmp x1, -9
    jeq L2
    jmp fail
L2:
    sub x1, 2
    prg x1
    cmp x1, -11
    jeq L3
    jmp fail
L3:
    sub x1, -2
    prg x1
    cmp x1, -9
    jeq L4
    jmp fail
L4:
    sub x1, -9
    prg x1
    cmp x1, 0
    jeq L5
    jmp fail
L5:
    mov x1, 0
    sub x1, 0xFFFFFFFFFFFFFFFF
    prg x1
    cmp x1, 1
    jeq L6
    jmp fail
L6:
    mov x1, 0
    sub x1, 0xFFFFFFFFFFFFFFFE
    prg x1
    cmp x1, 2
    jeq L7
    jmp fail
L7:
    mov x1, -1
    sub x1, 0xFFFFFFFFFFFFFFFF
    prg x1
    cmp x1, 0
    jeq L8
    jmp fail
L8:
    mov x1, -1
    sub x1, 0xFFFFFFFFFFFFFFFE
    prg x1
    cmp x1, 1
    jeq pass
    jmp fail
pass:
    mov  x0, 0x50
    call putchar
    mov  x0, 0x0A
    call putchar
    mov  x0, 0
    ret
fail:
    mov  x0, 0x46
    call putchar
    mov  x0, 0x0A
    call putchar
    mov  x0, 0
    ret
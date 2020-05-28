main:
    mov x5, 0
    blt  ops
    inc x5
    bgt  ops
    inc x5
    bge  ops
    inc x5
    ble  ops
    inc x5
    beq  ops
    inc x5
    bgt  ops
    inc x5
    mov x1, 0
    inc x5
    div  x0, 123
    inc x5
    div  x0, 1
    inc x5
    shr  x0, x1
    inc x5
    shr  x0, 123
    inc x5
    shl  x0, x1
    inc x5
    shl  x0, 123
    inc x5
    dec  x1 
    inc x5
    mul  x0, x1
    inc x5
    mul  x0, 123
    inc x5
    add  x0, x1
    inc x5
    add  x0, 123
    inc x5
    sub  x0, x1
    inc x5
    sub  x0, 123
    inc x5
    prg  x0
    inc x5
    prg  123
    inc x5
    prgi
    inc x5
    cmp x5, 23
    cmp 23, x5

    beq ops
    b   fail
    bne  ops
    bl   ops
ops:
    mov x0, 0   
    ret
fail:
    mov x0, 10 
    ret

main:
    mov x5, 0
    jlt  ops
    inc x5
    jgt  ops
    inc x5
    jge  ops
    inc x5
    jle  ops
    inc x5
    jeq  ops
    inc x5
    jgt  ops
    inc x5
    div  x0, x1
    inc x5
    div  x0, 123
    inc x5
    div  x0, 0
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

    jeq ops
    jmp  fail
    jne  ops
    call ops
ops:
    mov x0, 0   
    ret
fail:
    mov x0, 10 
    ret

main:
    jmp  ops
    jlt  ops
    jgt  ops
    jge  ops
    jle  ops
    jeq  ops
    jne  ops
    jgt  ops
    call ops
    div  x0, x1
    div  x0, 123
    shr  x0, x1
    shr  x0, 123
    shl  x0, x1
    shl  x0, 123
    inc  x0
    dec  x1 
    mul  x0, x1
    mul  x0, 123
    add  x0, x1
    add  x0, 123
    sub  x0, x1
    sub  x0, 123
    prg  x0
    prg  123
    prgi
ops:
    mov x0, 0   
    ret
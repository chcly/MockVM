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
    shr  x0, x1
    shl  x0, x1
    inc  x0
    dec  x1 
    mul  x0, x1
    add  x0, x1
    sub  x0, x1
    prg  x0
    prgi
ops:
    mov x0, 0   
    ret

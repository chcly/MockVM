main:
    mov x0, 0
top:
    cmp x0, 10
    bge done
    inc x0
    cmp x0, 5
    blt lt5
    prg 1
    b top
lt5:
    prg 0
    b top
done:
    mov x0, 0
    ret
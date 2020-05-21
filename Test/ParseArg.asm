main:
    mov x1, 1 
    cmp x1, 1
    prg x1
    jeq t1
    jmp fail
t1:
    mov x1, 0x0123456789ABCDEF 
    cmp x1, 81985529216486895
    prg x1
    jeq t2
    jmp fail
t2:
    mov x1, 0xFF 
    cmp x1, 255
    prg x1
    jeq t3
    jmp fail
t3:
    mov x1, 0b1100101011111001 
    cmp x1, 51961
    prg x1
    jeq success
    jmp fail
success:
    mov x0, 0
    ret
fail:
    mov x0, 10 ; any non zero code 
    ret


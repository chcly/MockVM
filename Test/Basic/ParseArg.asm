main:
    mov x1, 1 
    cmp x1, 1
    prg x1
    jeq t1
    jmp fail
; ---------------------------------------------------
t1:
    mov x1, 0x0123456789ABCDEF 
    cmp x1, 81985529216486895
    prg x1
    jeq t2
    jmp fail
; ---------------------------------------------------
t2:
    mov x1, 0xFF 
    cmp x1, 255
    prg x1
    jeq t3
    jmp fail
; ---------------------------------------------------
t3:
    mov x1, 0b1100101011111001 
    cmp x1, 51961
    prg x1
    jeq t4
    jmp fail
; ---------------------------------------------------
t4:
    mov x0, 0x02
    mov x1, 0x02
    mul x2, x0, x1
    cmp x2, 4
    prg x2
    jeq t5
    jmp fail
; ---------------------------------------------------
t5:
    mov x0, 0x02
    mov x1, 0x02
    div x2, x0, x1
    cmp x2, 1
    prg x2
    jeq t6
    jmp fail
; ---------------------------------------------------
t6:
    mov x0, 0x02
    mov x1, 0x02
    add x2, x0, x1
    cmp x2, 4
    prg x2
    jeq t7
    jmp fail
; ---------------------------------------------------
t7:
    mov x0, 0x02
    mov x1, 0x02
    sub x2, x0, x1
    cmp x2, 0
    prg x2
    jeq t8
    jmp fail
; ---------------------------------------------------
t8:
    mov x0, 1
    mov x1, 5
    shl x2, x0, x1
    cmp x2, 32
    prg x2
    jeq t9
    jmp fail
; ---------------------------------------------------
t9:
    mov x0, 32
    mov x1, 5
    shr x2, x0, x1
    cmp x2, 1
    prg x2
    jeq success
    jmp fail
; ---------------------------------------------------
success:
    mov x0, 0
    ret
fail:
    mov x0, 10 ; any non zero code 
    ret


main:
    mov x1, 1 
    cmp x1, 1
    prg x1
    beq t1
    b   fail
; ---------------------------------------------------
t1:
    mov x1, 0x0123456789ABCDEF 
    cmp x1, 81985529216486895
    prg x1
    beq t2
    b   fail
; ---------------------------------------------------
t2:
    mov x1, 0xFF 
    cmp x1, 255
    prg x1
    beq t3
    b   fail
; ---------------------------------------------------
t3:
    mov x1, 0b1100101011111001 
    cmp x1, 51961
    prg x1
    beq t4
    b   fail
; ---------------------------------------------------
t4:
    mov x0, 0x02
    mov x1, 0x02
    mul x2, x0, x1
    cmp x2, 4
    prg x2
    beq t5
    b   fail
; ---------------------------------------------------
t5:
    mov x0, 0x02
    mov x1, 0x02
    div x2, x0, x1
    cmp x2, 1
    prg x2
    beq t6
    b   fail
; ---------------------------------------------------
t6:
    mov x0, 0x02
    mov x1, 0x02
    add x2, x0, x1
    cmp x2, 4
    prg x2
    beq t7
    b   fail
; ---------------------------------------------------
t7:
    mov x0, 0x02
    mov x1, 0x02
    sub x2, x0, x1
    cmp x2, 0
    prg x2
    beq t8
    b   fail
; ---------------------------------------------------
t8:
    mov x0, 1
    mov x1, 5
    shl x2, x0, x1
    cmp x2, 32
    prg x2
    beq t9
    b   fail
; ---------------------------------------------------
t9:
    mov x0, 32
    mov x1, 5
    shr x2, x0, x1
    cmp x2, 1
    prg x2
    beq success
    b   fail
; ---------------------------------------------------
success:
    mov x0, 0
    ret
fail:
    mov x0, 10; any non zero code 
    ret


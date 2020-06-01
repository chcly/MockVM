; ----------------------------------------------------
                    .data
; ----------------------------------------------------
alphabet:  .asciz "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz !#$%&'()*+,-./:;<=>?@[\\]^_`{|}~\""
value:     .xword  0x0123456789ABCDEF
buffer:    .zero   64
; ----------------------------------------------------
                    .text
; ----------------------------------------------------
tobase: 
    adrp x9, value
    add  x1, x9, value
    adrp x8, alphabet
    adrp x7, buffer
    mov  x5, 0
tbl1:
    cmp  x1, 0
    ble  tbl2
    div  x2, x1, x0 
    mul  x3, x2, x0
    sub  x3, x1, x3
    mov  x1, x2
    ldrs x4, [x8, x3]
    strs x4, [x7, x5]
    inc  x5
    b    tbl1
tbl2:
    cmp  x5, 0
    beq  tbl3
    dec  x5
    ldrs x0, [x7, x5]
    bl   putchar
    b    tbl2
tbl3:
    mov  x0, 10
    bl   putchar
    ret
; ----------------------------------------------------
main:
    stp  sp, 8
    mov  x0, 2
ml1:
    cmp  x0, 96
    beq  ml2
    str  x0, [sp, 0]
    bl   tobase
    ldr  x0, [sp, 0]
    inc  x0
    b    ml1
ml2:
    mov  w0, 0
    ldp  sp, 8
    ret

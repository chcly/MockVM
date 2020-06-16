; -------------------------------------------
                .text
; -------------------------------------------
abs:
    cmp     x9, x8
    bge     absge
    sub     x9, x8, x9
    ret
absge:
    sub     x9, x9, x8
    ret
; -------------------------------------------
isqrt:                     ; X              = x0 
    mov     x1, 0          ; A              = x1
    mov     x2, x0         ; B              = x2
tsqrt:
    add     x3, x1, x2
    shr     x3, x3, 1      ; I = (A+B) / 2  = x3
    mul     x4, x3, x3     ; S = I*I        = x4
    mov     x9, x1
    mov     x8, x2
    bl      abs
    cmp     x9, 1
    bgt     snext
    b       dsqrt
snext:
    cmp     x4, x0         ; S > X
    bgt     sgtx
    mov     x1, x3         ; A = I
    b       tsqrt
sgtx:
    mov     x2, x3         ; B = I
    b       tsqrt
dsqrt:
    mov     x0, x3
    ret
; -------------------------------------------
main:
    mov     x7, 1
maint:  
    cmp     x7, 37
    bge     maind
    mov     x0, x7
    bl      isqrt
    prg     x0  
    inc     x7
    b       maint
maind:
    mov     x0, 0
    ret

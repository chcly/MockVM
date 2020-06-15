; ----------------------------------------------------
                    .data
; ----------------------------------------------------
alphabet:  .asciz "0123456789"
buffer:    .zero   64
; ----------------------------------------------------
                    .text
; ----------------------------------------------------
toBase10: 
    mov     x1, x0
    mov     x0, 10
    adrp    x8, alphabet
    adrp    x7, buffer
    mov     x5, 0
tbl1:
    cmp     x1, 0
    ble     tbl2
    div     x2, x1, x0 
    mul     x3, x2, x0
    sub     x3, x1, x3
    mov     x1, x2
    ldrs    x4, [x8, x3]
    strs    x4, [x7, x5]
    inc     x5
    b       tbl1
tbl2:
    mov     x6, x5
    cmp     x5, 0
    beq     tbl3
    dec     x5
    ldrs    x0, [x7, x5]
    bl      putchar
    b       tbl2
tbl3:
    mov     x0, 32
    bl      putchar
    ret
; ----------------------------------------------------------------------------
; x0 fact(n:x0) 
; ----------------------------------------------------------------------------
fact:
    mov     x1, 1
    mov     x2, x0
factst:
    cmp     x1, x2
    bge     facten
    mul     x0, x1, x0
    inc     x1
    b       factst
facten:
    ret

; ----------------------------------------------------------------------------
; x0 nChooseK(i:x0, j:x1) 
; ----------------------------------------------------------------------------
nChooseK:
    mov     x3, x0          ; x3 = i 
    mov     x4, x1          ; x4 = j
    sub     x5, x3, x4      ; x5 = i -j
    mov     x0, x3
    bl      fact
    mov     x9, x0
    mov     x0, x4
    bl      fact
    mov     x8, x0
    mov     x0, x5
    bl      fact
    mov     x7, x0
    mul     x8, x8, x7
    cmp     x8, 0
    bne     factdiv
    mov     x0, 1
    ret
factdiv:
    div     x0, x9, x8
    ret
; ----------------------------------------------------------------------------
; int pascals_triangle(total:x0) 
; ----------------------------------------------------------------------------
pascals_triangle:
    stp     sp, 32
    mov     x1, x0          ; total = x1
    mov     x0, 0           ; i = x0
ifst:
    cmp     x0, x1          ; i < total
    bge     ptridone
    add     x2, x0, 1       ; d = x2:[i + 1]
    mov     x3, 0           ; j = 0
jfst:
    cmp     x3, x2          ; j < d
    bge     ifen
    str     x0, [sp, 0x00]  ; i
    str     x1, [sp, 0x08]  ; total
    str     x2, [sp, 0x10]  ; d
    str     x3, [sp, 0x18]  ; j
    mov     x1, x3
    bl      nChooseK
    bl      toBase10
    ldr     x0, [sp, 0x00]  ; i
    ldr     x1, [sp, 0x08]  ; total
    ldr     x2, [sp, 0x10]  ; d
    ldr     x3, [sp, 0x18]  ; j
    inc     x3
    b       jfst
ifen:
    str     x0, [sp, 0x00]  ; i
    mov     x0, 10
    bl      putchar
    ldr     x0, [sp, 0x00]  ; i
    inc     x0
    b       ifst
ptridone:
    ldp     sp, 32
    ret
; ----------------------------------------------------------------------------
; int main() 
; ----------------------------------------------------------------------------
main:
    mov     x0, 16
    bl      pascals_triangle
    mov     x0, 0
    ret

; ----------------------------------------------------------------------------
; i64 i = 0
; while (i < 10)
;    i++
;    print i
; ----------------------------------------------------------------------------
main:
    mov x0, 0
    mov x1, 10
top:
    cmp x0, x1
    jge done
    inc x0
    prg x0
    jmp top
done:
    mov x0, 0
    ret

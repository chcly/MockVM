main:
    mov     x0, 0
top:
    cmp     x0, 10
    jeq     done
    inc     x0
    mov     x1, 5
    cmp     x0, x1
    jgt     px
    prgi
    jmp     top
px:
    prg     x0
    jmp     top
done:
    ret

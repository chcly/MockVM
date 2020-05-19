main:
    mov     x0, 0
    mov     x1, 100
top:
    cmp     x0, x1
    jeq     done
    prg     x0
    inc     x0
    jmp     top
done:
    ret

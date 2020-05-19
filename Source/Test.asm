main:
    mov     x0, 0
    mov     x1, 100
top:
    cmp     x0, x1
    jeq     done
    inc     x0
    trace
    jmp     top
done:
    ret
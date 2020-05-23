main:
    mov x0, 32
top:
    cmp x0, 127
    jeq done
    call putchar
    inc x0
    jmp top
done:
    mov x0, 0
    ret

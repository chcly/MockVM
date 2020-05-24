main:
    mov x0, ' '
top:
    cmp x0, '~'
    jgt done
    call putchar
    inc x0
    jmp top
done:
    mov x0, 0
    ret

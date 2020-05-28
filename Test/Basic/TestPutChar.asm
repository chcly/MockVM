main:
    mov x0, ' '
top:
    cmp x0, '~'
    bgt done
    bl putchar
    inc x0
    b top
done:
    mov x0, 0
    ret

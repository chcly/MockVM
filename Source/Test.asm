main:
    mov x0, 0
    mov x1, 100000000
top:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done:
    mov x0, 0
    ret

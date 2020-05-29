main:
    mov  x0, 0
    mov  x1, 0xFF
top:
    cmp  x0, x1
    bge  done
    inc  x0
    b    top
done:
    prg  x0
    mov  x0, 0
    ret
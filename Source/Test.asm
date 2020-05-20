main1:
    mov x0, 0
    mov x1, 1000
top1:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done1:
    mov x0, 0
    prgi
    ret
main2:
    mov x0, 0
    mov x1, 1000
top2:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done2:
    mov x0, 0
    prgi
    ret

main3:
    mov x0, 0
    mov x1, 1000
top3:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done3:
    mov x0, 0
    prgi
    ret



main4:
    mov x0, 0
    mov x1, 1000
top4:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done4:
    mov x0, 0
    prgi
    ret

main:
    mov x0, 0
    mov x1, 1000
top:
    cmp x0, x1
    jge done
    inc x0
    jmp top
done:
    mov x0, 0
    prgi
    ret

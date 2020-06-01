; -------------------------------------
                .data
; -------------------------------------
depth: .xword 95
; -------------------------------------
                .text
; -------------------------------------
recursive:
    stp  sp, 8
    dec  x1
    str  x1, [sp, 0]
    cmp  x1, 0
    blt  done
    beq  printnl
    b    skip
printnl:
    mov  x0, '\n'
    bl   putchar
skip:
    add  x0, x1, ' '
    bl   putchar
    bl   recursive
    ldr  x1, [sp, 0]
    add  x0, x1, ' '
    bl   putchar
done:
    ldp  sp, 8
    ret


main:
    adrp x1, depth
    add  x1, x1, depth
    bl   recursive
    mov  x0, '\n'
    bl   putchar
    mov  x0, 0
    ret
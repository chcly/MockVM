__abcdefghijklmnopqrstuvwzyz_0123456789__:
    mov x0, 0
    ret
__ABCDEFGHIJKLMNOPQRSTUVWZYZ_0123456789__:
    mov x0, 0
    ret
main:
    bl __abcdefghijklmnopqrstuvwzyz_0123456789__
    bl __ABCDEFGHIJKLMNOPQRSTUVWZYZ_0123456789__
    prg 1
    ret
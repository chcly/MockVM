__abcdefghijklmnopqrstuvwzyz_0123456789__:
    mov x0, 0
    ret
__ABCDEFGHIJKLMNOPQRSTUVWZYZ_0123456789__:
    mov x0, 0
    ret
main:
    call __abcdefghijklmnopqrstuvwzyz_0123456789__
    call __ABCDEFGHIJKLMNOPQRSTUVWZYZ_0123456789__
    prg 1
    ret
; ----------------------------------------------------
                .data
; ----------------------------------------------------
message1: .asciz "Hello World #1"
message2: .asciz "Hello World #2"
message3: .asciz "Hello World #3"
message4: .asciz "Hello World #4"
message5: .asciz "Hello World #5"
; ----------------------------------------------------
                .text
; ----------------------------------------------------
main:
    adrp  x0, message1
    bl    puts
    adrp  x0, message2
    bl    puts
    adrp  x0, message3
    bl    puts
    adrp  x0, message4
    bl    puts
    adrp  x0, message5
    bl    puts
    mov   x0, 0
    ret

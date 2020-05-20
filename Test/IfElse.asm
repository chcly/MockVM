main:
mov x0, 0
top:
cmp x0, 10
jge done
inc x0
cmp x0, 5
jlt lt5
prg 1
jmp top
lt5:
prg 0
jmp top
done:
mov x0, 0
ret
# ToyVM Instructions
1. [ToyVM Instructions](#toyvm-instructions)
    1. [Definitions](#definitions)
        1. [Registers](#registers)
        2. [Syntax](#syntax)
    2. [Basic Operations](#basic-operations)
        1. [ret](#ret)
        2. [mov R0, R1](#mov-r0-r1)
        3. [mov R0, V](#mov-r0-v)
        4. [mov PC, V](#mov-pc-v)
        5. [mov PC, R0](#mov-pc-r0)
        6. [inc R0](#inc-r0)
        7. [dec R0](#dec-r0)
    3. [Unconditional Branching](#unconditional-branching)
        1. [bl SYM](#bl-sym)
        2. [bl ADDR](#bl-addr)
        3. [b ADDR](#b-addr)
    4. [Conditional branching](#conditional-branching)
        1. [cmp R0, R1](#cmp-r0-r1)
        2. [cmp R0, V](#cmp-r0-v)
        3. [cmp V, R0](#cmp-v-r0)
        4. [beq ADDR](#beq-addr)
        5. [bne ADDR](#bne-addr)
        6. [ble ADDR](#ble-addr)
        7. [bge ADDR](#bge-addr)
        8. [blt ADDR](#blt-addr)
        9. [bgt ADDR](#bgt-addr)
    5. [Math operations](#math-operations)
        1. [op R0, R1|V](#op-r0-r1v)
        2. [op R0, R1|V, R2|V](#op-r0-r1v-r2v)
    6. [Stack operations](#stack-operations)
        1. [stp  SP, V](#stp-sp-v)
        2. [ldp  SP, V](#ldp-sp-v)
        3. [str R0, [SP, V]](#str-r0-sp-v)
        4. [ldr R0, [SP, V]](#ldr-r0-sp-v)

## Definitions

| Name | Description                                                                    |
|------|--------------------------------------------------------------------------------|
| R(n) | Is any value with the prefix b,w,l or x followed by a single digit [0-9]       |
| V    | Is any integer value in base 10, hexadecimal, binary, or a character constant. |
| PC   | Is the program counter.                                                        |
| SP   | The stack pointer.                                                             |
| SYM  | Refers to a symbol loaded from a dynamic library.                              |
| ADDR | Is a local address in the file referring to a label.                           |

### Registers

There are a total of 10 64 bit registers that can be used.

| Registers | Size   | Offset | C/C++    |
|:----------|--------|--------|----------|
| b(n)      | 8-bit  | 8      | char[8]  |
| l(n)      | 32-bit | 2      | short[4] |
| w(n)      | 16-bit | 4      | int[2]   |
| x(n)      | 64-bit | 0      | int64_t  |

### Syntax

```asm
                ; comment
main:           ; label
   mov x0, 0    ; op dest, src
top:
   cmp x0, 10
   bge done
   inc x0
   b top
done:
   mov x0, 0    ; return value is in x0
   ret
```

## Basic Operations

### ret

Removes a branch off the call stack and assigns an internal return code from register 0.

```asm
    mov x0, 0
    ret
```

### mov R0, R1

* R0 Is the destination register.  
* R1 Is the source register.  

```asm
    mov x0, x1
```

### mov R0, V

* R0 Is the destination register.  
* V Is the value to place in the register.  

```asm
    mov x0, 123
    mov x0, 0xFF
    mov x0, 0b1010
    mov x0, 'A'
```

### mov PC, V

* Explicitly sets the value of the program counter.

```asm
    mov pc, 1
    ret
```

### mov PC, R0

* Sets the value of the program counter to the value in R0.

```asm
    mov x0, 2
    mov pc, x0
    ret
```

### inc R0

Increments the register value by 1

```asm
    mov x0, 'A'
    inc x0
    bl putchar
```

### dec R0

Decrements the register value by 1

```asm
    mov x0, 'B'
    dec x0
    bl putchar
```

## Unconditional Branching

### bl SYM

Branches to a symbol defined in C/C++.

```asm
    mov x0, 'A'
    bl putchar
```

### bl ADDR

Branches to a local label and stores a link back to the calling instruction.

```asm
fn1:
    mov, 'A'
    bl   putchar
    ret

main:
    bl  fn1
    mov x0, 0
    ret
```

### b ADDR

Moves the current instruction to the location found in ADDR

```asm

L2:
    ret
L1:
    mov x0, 0
    b   L2
```

## Conditional branching

### cmp R0, R1

Subtracts R1 from R0 then then marks a flag based on the sign.

| Result | Flag |
|--------|------|
| R == 0 | Z    |
| R < 0  | L    |
| R > 0  | G    |

### cmp R0, V

Compares R0 with a constant.

### cmp V, R0

Compares a constant with R0.

### beq ADDR

Branch to the location found in ADDR if the Z flag is set.

```asm
    mov x0, 0
    cmp x0, 0
    beq L1:
    b   L2
L1:
    ret
L2:
    mov x0, -1
    ret
```

### bne ADDR

Branch to the location found in ADDR if the Z flag is not set.

```asm
    mov x0, 0
    cmp x0, 0
    bne L1
    b   L2
L1:
    mov x0, -1
    ret
L2:
    ret
```

### ble ADDR

Branch to the location found in ADDR if the Z flag is or the L flag is set.

### bge ADDR

Branch to the location found in ADDR if the Z flag is or the G flag is set.

### blt ADDR

Branch to the location found in ADDR if L flag is set.

### bgt ADDR

Branch to the location found in ADDR if G flag is set.

## Math operations

Where op is one of the following:

* add
* sub
* mul
* div
* shr
* shl

### op R0, R1|V

Preforms the operation on R0 and R1 into R0.

```asm
    mov x0, 2
    mov x1, 2
    add x0, x1
    add x0, 2
    ret
```

### op R0, R1|V, R2|V

Preforms the operation on the R1 and R2 registers into R0

```asm
    mov x0, 2
    mov x1, 2
    add x3, x0, x1
    add x4, x0, 2
    ret
```

## Stack operations

### stp  SP, V

Stores V bytes of stack space.

```asm
    stp sp, 16
```

### ldp  SP, V

Pops V bytes off the stack.

```asm
    ldp sp, 16
```

### str R0, [SP, V]

Stores R0 on the stack at the supplied offset found in V.

```asm
    stp sp, 8
    mov x0, 100
    str x0, [sp, 0]
    mov x0, 0
    ldp sp, 8
```

### ldr R0, [SP, V]

Loads into R0 the value found on the stack at the offset found in V.

```asm
    stp sp, 8
    mov x0, 100
    str x0, [sp, 0]
    mov x0, 0
    ldr x0, [sp, 0]
    ldp sp, 8
```

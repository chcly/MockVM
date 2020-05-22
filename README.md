# ToyVM

Is a custom assembly compiler and a virtual machine to execute the compiled code.
The implemented assembly is a mixture of ARM and Intel operations, but it tends towards a more ARM like architecture.
As of right now, there are two main programs in the repo.

* tcom
* tvm

## tcom

Is the assembly compiler.

### tcom Usage

```txt
tcom <options> <input file>

   options:
      -h show this message.
      -o output file.
```

## Syntax

```asm
                ; comment
main:           ; label
   mov x0, 0    ; op dest, src
top:
   cmp x0, 10
   jge done
   inc x0
   jmp top
done:
   mov x0, 0    ; return value is in x0
   ret
```

Each program needs at least one label called main.

### Registers

It has a total of ten 64 bit registers that it can use.

| Registers | Size   | Offset | status |
|:----------|--------|--------|--------|
| x(n)      | 64-bit | 0      | done   |
| l(n)      | 32-bit | 2      | todo   |
| w(n)      | 16-bit | 4      | todo   |
| b(n)      | 8-bit  | 8      | todo   |

Each register is a union, so for ten 64 bit registers, there are a total of 20 32-bit registers,
40 16 bit registers, and 80 8-bit registers.

#### Current op codes

| Opcode | Usage             | Op1 | Op2 | Op3(Ex) | Description                                          |
|:-------|:------------------|:---:|:---:|:-------:|------------------------------------------------------|
| mov    | mov dest, source  |  R  | R/V |         | moves the source value into the destination register |
| ret    | ret               |     |     |         | return values should be placed in the x0 register    |
| cmp    | cmp Op1, Op2      |  R  | R/V |         | does a logical comparison of Op1, and Op2            |
| inc    | inc Op1           |  R  |     |         | increments Op1 by 1                                  |
| dec    | dec Op1           |  R  |     |         | decrements Op1 by 1                                  |
| jmp    | jmp label         |  A  |     |         | unconditional jump.                                  |
| jeq    | jeq label         |  A  |     |         | jump if the E flag is set.                           |
| jne    | jne label         |  A  |     |         | jump if the E flag is not set.                       |
| jgt    | jgt label         |  A  |     |         | jump if the G flag is set.                           |
| jlt    | jlt label         |  A  |     |         | jump if the L flag is set.                           |
| jge    | jge label         |  A  |     |         | jump if the G or E flags are set.                    |
| jle    | jle label         |  A  |     |         | jump if the L or E flags are set.                    |
| add    | add Op1, Op2, Op3 |  R  | R/V |   R/V   | add Op1 and Op2 and store the result in Op1          |
| sub    | sub Op1, Op2, Op3 |  R  | R/V |   R/V   | subtract Op1 and Op2 and store the result in Op1     |
| mul    | mul Op1, Op2, Op3 |  R  | R/V |   R/V   | multiply Op1 and Op2 and store the result in Op1     |
| div    | div Op1, Op2, Op3 |  R  | R/V |   R/V   | divide Op1 and Op2 and store the result in Op1       |
| shr    | shr Op1, Op2, Op3 |  R  | R/V |   R/V   | shift Op1 right by Op2 and store the result in Op1   |
| shl    | shl Op1, Op2, Op3 |  R  | R/V |   R/V   | shift Op1 left by Op2 and store the result in Op1    |
| prg    | prg Op1           | R/V |     |         | prints the operand to stdout.                        |
| prgi   | prgi              |     |     |         | prints the contents of all registers to stdout.      |

## tvm

tvm runs the executable generated from tcom.

### tvm Usage

```txt
tvm <options> <program_path>

   options:
      -h display this message
      -t display execution time
```

## Building

Building with CMAKE.

```txt
To compile the basic tests add -DBUILD_TEST=ON to the CMake command line.

mkdir Build
cd Build
cmake ..
```

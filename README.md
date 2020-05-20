# ToyVM

Is what the name suggests. It is a simple assembly compiler and a virtual machine to execute the binary. The assembly language is a mixture of ARM and Intel ish operations. As of right now, there are two main programs in the repo.

* tcom
* tvm

1. [ToyVM](#toyvm)
    1. [tcom](#tcom)
    2. [tvm](#tvm)
    3. [Building](#building)

## tcom

Takes the assembly source and compiles the program.

It has a total of ten 64 bit registers that it can use.
Each register is a union, so for ten 64 bit registers, there are a total of 20 32-bit registers, 40 16 bit registers, and 80 8-bit registers.

| Registers | Size   | Offset | status |
|:----------|--------|--------|--------|
| x(n)      | 64-bit | 0      | done   |
| l(n)      | 32-bit | 2      | todo   |
| w(n)      | 16-bit | 4      | todo   |
| b(n)      | 8-bit  | 8      | todo   |



| Opcode | Usage            | Operand1 | Operand2     | Description                                       |
|:-------|:-----------------|:---------|:-------------|---------------------------------------------------|
| mov    | mov dest, source | reg      | reg or value |                                                   |
| ret    | ret              |          |              | return values should be placed in the 0 registers |
| cmp    | cmp op1, op2     | reg      | reg or value |                                                   |
| jmp    | jmp label        | addr     |              |                                                   |
| jeq    | jeq label        | addr     |              |                                                   |
| jne    | jne label        | addr     |              |                                                   |
| jgt    | jgt label        | addr     |              |                                                   |
| jlt    | jlt label        | addr     |              |                                                   |
| jge    | jge label        | addr     |              |                                                   |
| jle    | jle label        | addr     |              |                                                   |
| prg    | prg op1          | addr     |              | prints the operand to stdout.                     |
| prgi   | prgi             |          |              | prints the contents of all registers to stdout.   |

## tvm

tvm executes the file compiled with tcom.

## Building

Building with CMAKE.

```txt
mkdir Build
cd Build
cmake ..
```

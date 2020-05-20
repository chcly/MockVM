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


| Opcode | Usage            | Operand1 | Operand2     | Description                                           |
|:-------|:-----------------|:---------|:-------------|-------------------------------------------------------|
| mov    | mov dest, source | reg      | reg or value | moves the source value into the destination register. |
| ret    | ret              |          |              | return values should be placed in the 0 registers.    |
| cmp    | cmp op1, op2     | reg      | reg or value | Does a logical comparison  of op1, and op2            |
| jmp    | jmp label        | addr     |              | unconditional jump.                                   |
| jeq    | jeq label        | addr     |              | jump if the E flag is set.                            |
| jne    | jne label        | addr     |              | jump if the E flag is not set.                        |
| jgt    | jgt label        | addr     |              | jump if the G flag is set.                            |
| jlt    | jlt label        | addr     |              | jump if the L flag is set.                            |
| jge    | jge label        | addr     |              | jump if the G or E flags are set.                     |
| jle    | jle label        | addr     |              | jump if the L or E flags are set.                     |
| add    | add op1, op2     | reg      | reg or value | add op1, and op2 and store the result in op1          |
| sub    | sub op1, op2     | reg      | reg or value | subtract op1, and op2 and store the result in op1     |
| mul    | mul op1, op2     | reg      | reg or value | multiply op1, and op2 and store the result in op1     |
| div    | div op1, op2     | reg      | reg or value | divide  op1, and op2 and store the result in op1      |
| prg    | prg op1          | addr     |              | prints the operand to stdout.                         |
| prgi   | prgi             |          |              | prints the contents of all registers to stdout.       |

## tvm

tvm runs the executable generated from tcom.

## Building

Building with CMAKE.

```txt
mkdir Build
cd Build
cmake ..
```

To compile the basic tests add -DBUILD_TESTS=ON to the CMake command line or switch the option to on from the CMake GUI.  

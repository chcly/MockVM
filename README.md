# ToyVM

ToyVM is a custom assembly compiler and a simple virtual machine/interpreter to execute the compiled code. It runs on Windows, Linux, and OSX. It supports linking c/c++ dynamic libraries through a simple plugin interface.

The exported symbols are accessible with the bl opcode.

```asm
bl exported_symbol
```

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
      -l link library.
      -d disable full path when reporting errors.
      -m print the module path and exit.
```

* -l Links a shared library into the file.
* -m Displays the location of the shared library folder.
* -d Is used in the tests to prevent full path names from being reported, which would cause them to fail.

## Syntax

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

| Opcode | Usage            | o1  | o2  | o3(Ex) | Description                                          |
|:-------|:-----------------|:---:|:---:|:------:|------------------------------------------------------|
| mov    | mov dest, source |  R  | R/V |        | moves the source value into the destination register |
| ret    | ret              |     |     |        | return values should be placed in the x0 register    |
| cmp    | cmp o1, o2       | R/V | R/V |        | does a logical comparison of o1, and o2              |
| inc    | inc o1           |  R  |     |        | increments o1 by 1                                   |
| dec    | dec o1           |  R  |     |        | decrements o1 by 1                                   |
| b      | b label          |  A  |     |        | branch to label.                                     |
| bl     | bl label         |  A  |     |        | branch with link.                                    |
| beq    | beq label        |  A  |     |        | branch if the E flag is set.                         |
| bne    | bne label        |  A  |     |        | branch if the E flag is not set.                     |
| bgt    | bgt label        |  A  |     |        | branch if the G flag is set.                         |
| blt    | blt label        |  A  |     |        | branch if the L flag is set.                         |
| bge    | bge label        |  A  |     |        | branch if the G or E flags are set.                  |
| ble    | ble label        |  A  |     |        | branch if the L or E flags are set.                  |
| add    | add o1, o2, o3   |  R  | R/V |  R/V   | add o1 and o2 and store the result in o1             |
| sub    | sub o1, o2, o3   |  R  | R/V |  R/V   | subtract o1 and o2 and store the result in o1        |
| mul    | mul o1, o2, o3   |  R  | R/V |  R/V   | multiply o1 and o2 and store the result in o1        |
| div    | div o1, o2, o3   |  R  | R/V |  R/V   | divide o1 and o2 and store the result in o1          |
| shr    | shr o1, o2, o3   |  R  | R/V |  R/V   | shift o1 right by o2 and store the result in o1      |
| shl    | shl o1, o2, o3   |  R  | R/V |  R/V   | shift o1 left by o2 and store the result in o1       |
| prg    | prg o1           | R/V |     |        | prints the operand to stdout.                        |
| prgi   | prgi             |     |     |        | prints the contents of all registers to stdout.      |

## tvm

tvm runs the executable file generated from tcom.

### tvm Usage

```txt
tvm <options> <program_path>

   options:
      -h display this message.
      -t display execution time.
      -m print the module path and exit.
```

## Building

Building with CMAKE.

```txt
mkdir Build
cd Build
cmake ..
```

Testing and Installing.

```txt
mkdir Build
cd Build
cmake -DToyVM_INSTALL_PATH=<some install directory> -DBUILD_TEST=ON ..
```

By default the executables are copied into ${CMAKE_BINARY_DIR}/bin and tested from that directory.

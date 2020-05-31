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

It has a total of 30 64 bit registers that may be used.

| Registers | Size   | Offset |
|:----------|--------|--------|
| b(n)      | 8-bit  | 8      |
| l(n)      | 32-bit | 2      |
| w(n)      | 16-bit | 4      |
| x(n)      | 64-bit | 0      |

#### Current op codes

| Opcode | Usage          | o1  | o2  | o3(Ex) | Description                                                                                                             |
|:-------|:---------------|:---:|:---:|:------:|-------------------------------------------------------------------------------------------------------------------------|
| mov    | mov o1, o2     |  R  | R/V |        | Moves the o2 into the o1 register.                                                                                      |
| ret    | ret            |     |     |        | Returns to the calling branch or exits the program if the call stack is empty.                                          |
| cmp    | cmp o1, o2     | R/V | R/V |        | Subtracts o1 and o2 and sets the Z, G, L flags.                                                                         |
| inc    | inc o1         |  R  |     |        | Increments o1 by 1.                                                                                                     |
| dec    | dec o1         |  R  |     |        | Decrements o1 by 1.                                                                                                     |
| b      | b label        |  A  |     |        | Branch to local label (will not work for calls and does not store the location that it branched from).                  |
| bl     | bl label       |  A  |     |        | Branch with link (used to call symbols or branch to labels. Returns to previous branch location when ret is processed). |
| beq    | beq label      |  A  |     |        | Branch if the Z flag is set.                                                                                            |
| bne    | bne label      |  A  |     |        | Branch if the Z flag is not set.                                                                                        |
| bgt    | bgt label      |  A  |     |        | Branch if the G flag is set.                                                                                            |
| blt    | blt label      |  A  |     |        | Branch if the L flag is set.                                                                                            |
| bge    | bge label      |  A  |     |        | Branch if the G or Z flags are set.                                                                                     |
| ble    | ble label      |  A  |     |        | Branch if the L or Z flags are set.                                                                                     |
| add    | add o1, o2, o3 |  R  | R/V |  R/V   | Add o1 and o2 and store the result in o1.                                                                               |
| sub    | sub o1, o2, o3 |  R  | R/V |  R/V   | Subtract o1 and o2 and store the result in o1.                                                                          |
| mul    | mul o1, o2, o3 |  R  | R/V |  R/V   | Multiply o1 and o2 and store the result in o1.                                                                          |
| div    | div o1, o2, o3 |  R  | R/V |  R/V   | Divide o1 and o2 and store the result in o1.                                                                            |
| shr    | shr o1, o2, o3 |  R  | R/V |  R/V   | Shift o1 right by o2 and store the result in o1.                                                                        |
| shl    | shl o1, o2, o3 |  R  | R/V |  R/V   | Shift o1 left by o2 and store the result in o1.                                                                         |
| adrp   | adrp o1, label |  R  |  A  |        | Loads the base address of the label into the specified register.                                                        |
| prg    | prg o1         | R/V |     |        | Prints the operand to stdout.                                                                                           |
| prgi   | prgi           |     |     |        | Prints the contents of all registers to stdout.                                                                         |

| Flags | Meaning    |  |
|:------|------------|--|
| Z     | o1-o2 == 0 |  |
| G     | o1-o2 > 0  |  |
| L     | o1-o2 < 0  |  |

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

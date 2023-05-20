# MockVM

Is a mock assembly compiler and virtual machine/interpreter.

+ It has been tested on Windows, Linux, OSX.
+ Supports linking c/c++ libraries through a simple plugin interface.

## Design

It is composed of three main programs.
### tcom

Is the compiler that transforms an input file into a mock binary.

#### Usage

```txt
tcom <options> <input file>

   options:
      -h show this message.
      -o output file.
      -l link library.
      -d disable full path when reporting errors.
      -m print the module path and exit.
```

### tvm

Is the program that interprets the binary and executes the instructions.

#### Usage

```txt
tvm <options> <program_path>

   options:
      -h display this message.
      -t display execution time.
      -m print the module path and exit.
```

### tdbg

Is the command line debugger.
It allows stepping, breakpoints, print-outs, and a view of the data section.

#### Compiling

It is disabled by default because the Linux and Mac versions use ncurses for the display.

+ `-DBUILD_DBG=ON` will enable it.
+ The ncurses dependency can be obtained using: `sudo apt-get install libncurses-dev`.
+ It was tested with version `(6.2-0ubuntu2)`.

You can find a screen capture of it running [here](Capture.gif).

#### Usage

```txt
tdbg <options> <program_path>

  options:
      -h display this message.

  Usage:
       Q - display exit screen.
       C - continue until the next breakpoint.
       B - add a break point on the current instruction.
       Down Arrow - step one instruction.

```


### Documentation

Documentation on the instructions may also be found [here](Codes.md).

## Testing

The Test directory is setup to work with [googletest](https://github.com/google/googletest).

## Building

Building with CMake and Make.

```sh
mkdir build
cd build
cmake .. -DBUILD_TEST=ON -DBUILD_DBG=ON
make
```

### Optional defines

| Option             | Description                                    | Default |
| :----------------- | :--------------------------------------------- | :-----: |
| ToyVM_INSTALL_PATH | Specify the directory to install the programs. |         |
| BUILD_TEST         | Build the test programs.                       |   OFF   |
| BUILD_DBG          | Compile the debugger.                          |   OFF   |


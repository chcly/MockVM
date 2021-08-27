# ToyVM

ToyVM is a mock assembly compiler and virtual machine/interpreter. It has been tested on Windows, Linux, OSX, and it supports linking c/c++ libraries through a simple plugin interface.


It is composed of three main programs.

* tcom
* tvm
* tdbg

## tcom

Is the assembly compiler that transforms the input file into a mock binary.


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

## tvm

tvm is the program that interprets the mock binary code and executes the instructions. 

### tvm Usage

```txt
tvm <options> <program_path>

   options:
      -h display this message.
      -t display execution time.
      -m print the module path and exit.
```

## tdbg

tdbg is a visual command line debugger. It allows stepping, breakpoints and print-outs of the current register state as well as the current state of the data section.   


### Enabling 

It is disabled by default because the Linux and Mac versions use ncurses for the display which may or may not be installed. ```-DBUILD_DBG=ON``` will enable it.

It can be obtained using:
```txt
sudo apt-get install libncurses-dev
```
and has been tested with ```(6.2-0ubuntu2).```


You can find a screen capture of it running [here](Capture.gif).


### tdbg Usage

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


## Building

Building with CMAKE and make.

```txt
mkdir Build
cd Build
cmake ..
make
```

Testing and Installing.

```txt
mkdir Build
cd Build
cmake -DToyVM_INSTALL_PATH=<some install directory> -DBUILD_TEST=ON ..
make
make install
```

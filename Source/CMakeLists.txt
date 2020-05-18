


set(SRC 
    Compiler.cpp
    BlockReader.cpp
    BlockReader.h
    Parser.cpp
    Parser.h
    Instruction.h
    Decl.h

    Test.asm
)


add_executable(tvm ${SRC})


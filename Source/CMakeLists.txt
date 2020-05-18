


set(SRC 
    Compiler.cpp
    BlockReader.cpp
    BinaryWriter.cpp
    Parser.cpp

    BlockReader.h
    BinaryWriter.h
    Parser.h
    Instruction.h
    Decl.h

    Test.asm
)


add_executable(tvm ${SRC})


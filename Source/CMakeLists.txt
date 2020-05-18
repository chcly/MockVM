

add_executable(tcom 
    BlockReader.h
    BinaryWriter.h
    Parser.h
    Instruction.h
    Decl.h
    BlockReader.cpp
    BinaryWriter.cpp
    Parser.cpp
    Compiler.cpp
)

add_executable(tvm 
    BlockReader.h
    BinaryReader.h
    Program.h
    Instruction.h
    Decl.h

    BlockReader.cpp
    BinaryReader.cpp
    Program.cpp
    VM.cpp
)


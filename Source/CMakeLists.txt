


set(SRC 
    Compiler.cpp
    BlockReader.cpp
    BlockReader.h
    Parser.cpp
    Parser.h
    Decl.h

    StateTable.inl
    Test.asm
)


add_executable(tvm ${SRC})


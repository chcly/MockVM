

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

set(INSTALL_PATH CACHE STRING "")

if (NOT INSTALL_PATH STREQUAL "")
    message(STATUS "INSTALL ==> ${INSTALL_PATH}")
    add_custom_command(TARGET tcom 
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tcom> 
                       "${INSTALL_PATH}/$<TARGET_FILE_BASE_NAME:tcom>$<TARGET_FILE_SUFFIX:tcom>")

    add_custom_command(TARGET tvm 
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tvm> 
                       "${INSTALL_PATH}/$<TARGET_FILE_BASE_NAME:tvm>$<TARGET_FILE_SUFFIX:tvm>")
endif()

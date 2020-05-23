
set(CommonSource
    BlockReader.cpp
    BinaryWriter.cpp
    Parser.cpp
    BlockReader.cpp
    Program.cpp
    SymbolUtils.cpp
)


set(CommonHeader
    BlockReader.h
    BinaryWriter.h
    Parser.h
    Declarations.h
    BlockReader.h
    Program.h
    SymbolUtils.h
    ../README.md
)

add_library(libtvm  ${CommonSource} ${CommonHeader})

if (NOT WIN32)
    target_link_libraries(libtvm dl)
endif()


add_executable(tcom 
    Compiler.cpp
)
target_link_libraries(tcom libtvm)

add_executable(tvm 
    VM.cpp
)
target_link_libraries(tvm libtvm)

subdirs(stdlib)
target_link_libraries(libtvm std)



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

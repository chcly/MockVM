macro(add_test Target)
    set(Input "${CMAKE_SOURCE_DIR}/Test/${ARGN}")


    add_custom_target(${Target} ALL
        COMMAND tcom -o ${Target} ${Input}
        COMMAND tvm ${Target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling ${ARGN}"
        SOURCES ${Input}
    )

endmacro(add_test)
add_test(ForLoop       ForLoop.asm)
add_test(IfElse        IfElse.asm)
add_test(PrintRegister PrintRegister.asm)
add_test(Factorial     Factorial.asm)
add_test(Compact       Compact.asm)
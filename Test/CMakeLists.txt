
macro(add_test Target)

    set(Input "${CMAKE_SOURCE_DIR}/Test/${ARGN}")



    add_custom_target(${Target} ALL
        COMMAND tcom -o ${Target} ${Input}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling ${ARGN}"
        SOURCES ${Input}
    )

    add_custom_target(${Target}Run ALL
        COMMAND tvm ${Target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running ${ARGN}"
    )
endmacro(add_test)


add_test(Test1 ForLoop.asm)
add_test(Test2 IfElse.asm)

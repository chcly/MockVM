

macro(add_compile_tests OUT)
    # Tests to make sure that everything 
    # compiles and executes

    foreach (it ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)
        
        set(GEN_FILE ${CMAKE_BINARY_DIR}/${GENNAME})
        list(APPEND ${OUT} ${GEN_FILE})
        list(APPEND ${OUT} ${ASMFILE})

        source_group("Assembly" FILES ${ASMFILE})
        source_group("Generated" FILES ${GEN_FILE})

        add_custom_command(
            OUTPUT ${GEN_FILE}
            COMMAND tcom -o ${GEN_FILE} ${ASMFILE}
            COMMAND tvm ${GEN_FILE} -t
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm
            COMMENT "${ASMNAME}"
        )
    endforeach(it)


endmacro(add_compile_tests)


set(TestFiles
    CRLF.asm
    CR.asm
    LF.asm
    Keywords.asm
    Comment.asm
    ParseArg.asm
    ForLoop.asm
    PrintRegister.asm
    IfElse.asm
    Factorial.asm
    Compact.asm
    Label.asm
)
add_compile_tests(OutFiles ${TestFiles})

set(SRC_ALL
    Catch2.h
    catch/catch.hpp
    Main.cpp
    BlockReader.cpp
    ${OutFiles}
    ${ToyVM_BINARY_DIR}/TestConfig.h
)


include_directories(../Source ${ToyVM_BINARY_DIR})

add_executable(tvmtest ${SRC_ALL})
target_link_libraries(tvmtest libtvm)
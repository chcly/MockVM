

macro(add_compile_tests OUT Group)
    
    # Tests to make sure that everything 
    # compiles and executes

    foreach (it ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)

        set(GEN_FILE     ${CMAKE_BINARY_DIR}/${GENNAME})
        set(GEN_FILE_ANS ${CMAKE_BINARY_DIR}/${GENNAME}.ans)
        set(GEN_FILE_EXP ${CMAKE_CURRENT_SOURCE_DIR}/${Group}/${GENNAME}.ans)

        list(APPEND ${OUT} ${GEN_FILE})
        list(APPEND ${OUT} ${ASMFILE})
        list(APPEND ${OUT} ${GEN_FILE_EXP})

        source_group("Assembly\\${Group}"  FILES ${ASMFILE})
        source_group("Generated\\${Group}" FILES ${GEN_FILE})
        source_group("Expected\\${Group}"  FILES ${GEN_FILE_EXP})

        add_custom_command(
            OUTPUT ${GEN_FILE}
            COMMAND tcom -o ${GEN_FILE} ${ASMFILE}
            COMMAND tvm ${GEN_FILE} > ${GEN_FILE_ANS}
            COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${GEN_FILE_ANS} ${GEN_FILE_EXP} 
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm
            COMMENT "Testing ${ASMNAME}"
        )

    endforeach(it)

endmacro(add_compile_tests)


macro(add_temp_test OUT Group)
    
    # Tests to make sure that everything 
    # compiles and executes

    foreach (it ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)

        set(GEN_FILE ${CMAKE_CURRENT_BINARY_DIR}/${GENNAME})
        set(GEN_FILE_ANS ${CMAKE_BINARY_DIR}/${GENNAME}.ans)

        list(APPEND ${OUT} ${GEN_FILE})
        list(APPEND ${OUT} ${ASMFILE})

        source_group("Assembly\\${Group}"  FILES ${ASMFILE})
        source_group("Generated\\${Group}" FILES ${GEN_FILE})

        add_custom_command(
            OUTPUT ${GEN_FILE}
            COMMAND tcom -d -o ${GEN_FILE} ${ASMFILE} 
            COMMAND tvm ${GEN_FILE} -t
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm
            COMMENT "${ASMNAME}"
        )
    endforeach(it)

endmacro(add_temp_test)



macro(add_test_dump_err OUT Group)
        

    foreach (it ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)

        set(GEN_FILE_ANS ${CMAKE_BINARY_DIR}/${GENNAME}.ans)
        set(GEN_FILE_EXP ${CMAKE_CURRENT_SOURCE_DIR}/${Group}/${GENNAME}.ans)

        list(APPEND ${OUT} ${GEN_FILE_ANS})
        list(APPEND ${OUT} ${ASMFILE})
        list(APPEND ${OUT} ${GEN_FILE_EXP})
        
        source_group("Assembly\\${Group}"  FILES ${ASMFILE})
        source_group("Generated\\${Group}" FILES ${GEN_FILE_ANS})
        source_group("Expected\\${Group}"  FILES ${GEN_FILE_EXP})

        add_custom_command(
            OUTPUT ${GEN_FILE_ANS}
            COMMAND tcom -d -o ${GEN_FILE} ${ASMFILE} > ${GEN_FILE_ANS}  ||
                    ${CMAKE_COMMAND} -E compare_files --ignore-eol ${GEN_FILE_ANS} ${GEN_FILE_EXP}
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm
            COMMENT "${ASMNAME}"
        )
    endforeach(it)
endmacro(add_test_dump_err)



set(TestFiles_1
    Basic/CRLF.asm
    Basic/CR.asm
    Basic/LF.asm
    Basic/Keywords.asm
    Basic/Comment.asm
    Basic/ParseArg.asm
    Basic/ForLoop.asm
    Basic/PrintRegister.asm
    Basic/IfElse.asm
    Basic/Factorial.asm
    Basic/Compact.asm
    Basic/Label.asm
    Basic/TestPutChar.asm
)

set(TestFiles_2
    Exec/Add1.asm
    Exec/Add2.asm
    Exec/Add3.asm
    Exec/Sub1.asm
    Exec/Sub2.asm
    Exec/Mul1.asm
    Exec/Div1.asm
)


set(TestFiles_3
    Errors/Err1.asm
    Errors/Err2.asm
    Errors/Err3.asm
)


#set(TestFiles_0
#)
#add_temp_test(OutFiles_0 Errors ${TestFiles_0})


add_compile_tests(OutFiles_1 Basic ${TestFiles_1})
add_compile_tests(OutFiles_2 Exec  ${TestFiles_2})
add_test_dump_err(OutFiles_3 Errors ${TestFiles_3})

set(SRC_ALL
    Catch2.h
    catch/catch.hpp
    Main.cpp
    BlockReader.cpp
    ${OutFiles_0}
    ${OutFiles_1}
    ${OutFiles_2}
    ${OutFiles_3}
    ${ToyVM_BINARY_DIR}/TestConfig.h
)


include_directories(../Source ${ToyVM_BINARY_DIR})

add_executable(tvmtest ${SRC_ALL})
target_link_libraries(tvmtest libtvm)


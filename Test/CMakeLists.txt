# -----------------------------------------------------------------------------
#   Copyright (c) 2020 Charles Carley.
#
#   This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
# ------------------------------------------------------------------------------
subdirs(fcmp)
set(tcom ${ToyVM_BIN_DIR}/tcom)
set(tvm  ${ToyVM_BIN_DIR}/tvm)
set(fcmp  ${ToyVM_BIN_DIR}/fcmp)

macro(add_compile_tests OUT Group)
    foreach (it IN ITEMS ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)

        set(GEN_FILE     ${CMAKE_BINARY_DIR}/${GENNAME})
        set(GEN_FILE_ANS ${CMAKE_BINARY_DIR}/${GENNAME}.ans)
        set(GEN_FILE_EXP ${CMAKE_CURRENT_SOURCE_DIR}/${Group}/${GENNAME}.ans)

        list(APPEND ${OUT} ${GEN_FILE})
        list(APPEND ${OUT} ${ASMFILE})
        list(APPEND ${OUT} ${GEN_FILE_EXP})
        list(APPEND ${OUT} ${GEN_FILE_ANS})

        set_source_files_properties(${GEN_FILE_ANS} GENERATED)
        source_group("Test\\${Group}\\Assembly"    FILES ${ASMFILE})
        source_group("Test\\${Group}\\Generated"   FILES ${GEN_FILE})
        source_group("Result\\${Group}\\Expected"  FILES ${GEN_FILE_EXP})
        source_group("Result\\${Group}\\Actual"    FILES ${GEN_FILE_ANS})

        add_custom_command(
            OUTPUT ${GEN_FILE}
            COMMAND ${tcom} -o ${GEN_FILE} ${ASMFILE}
            COMMAND ${tvm} ${GEN_FILE} > ${GEN_FILE_ANS}
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm fcmp std
            COMMENT "${ASMNAME}"
            VERBATIM
        )

        add_custom_command(
            OUTPUT ${GEN_FILE}.txt
            MAIN_DEPENDENCY ${GEN_FILE_ANS}
            COMMAND ${fcmp} ${GEN_FILE_ANS} ${GEN_FILE_EXP} > ${GEN_FILE}.txt
            DEPENDS tcom tvm fcmp std ${GEN_FILE}
            VERBATIM
            COMMENT ""
        )
    endforeach(it)
endmacro(add_compile_tests)


macro(add_temp_test OUT Group)
    foreach (it IN ITEMS ${ARGN})
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
            COMMAND ${tcom} -d -o ${GEN_FILE} ${ASMFILE} 
            COMMAND ${tvm} ${GEN_FILE} -t
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm fcmp std
            COMMENT "${ASMNAME}"
        )
    endforeach(it)
endmacro(add_temp_test)


macro(add_test_dump_err OUT Group)
    foreach (it IN ITEMS ${ARGN})
        get_filename_component(ASMFILE ${it}      ABSOLUTE)
        get_filename_component(GENNAME ${ASMFILE} NAME_WE)
        get_filename_component(ASMNAME ${it}      NAME)

        set(GEN_FILE_ANS ${CMAKE_BINARY_DIR}/${GENNAME}.ans)
        set(GEN_FILE_EXP ${CMAKE_CURRENT_SOURCE_DIR}/${Group}/${GENNAME}.ans)

        list(APPEND ${OUT} ${GEN_FILE_ANS})
        list(APPEND ${OUT} ${ASMFILE})
        list(APPEND ${OUT} ${GEN_FILE_EXP})
        list(APPEND ${OUT} ${GEN_FILE_ANS})
        set_source_files_properties(${GEN_FILE_ANS} GENERATED)

        source_group("Test\\${Group}\\Assembly"    FILES ${ASMFILE})
        source_group("Test\\${Group}\\Generated"   FILES ${GEN_FILE})
        source_group("Result\\${Group}\\Expected"  FILES ${GEN_FILE_EXP})
        source_group("Result\\${Group}\\Actual"    FILES ${GEN_FILE_ANS})

        add_custom_command(
            OUTPUT ${GEN_FILE_ANS}
            COMMAND ${tcom} -d -o ${GEN_FILE} ${ASMFILE} > ${GEN_FILE_ANS}  ||
                    ${fcmp} ${GEN_FILE_ANS} ${GEN_FILE_EXP} > ${GEN_FILE}.txt
            MAIN_DEPENDENCY ${ASMFILE}
            DEPENDS tcom tvm fcmp std
            COMMENT "${ASMNAME}"
        )
    endforeach(it)
endmacro(add_test_dump_err)

set(TestFiles_1
    Basic/CRLF.asm
    Basic/CR.asm
    Basic/LF.asm
    #Basic/Bug1.asm
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
    Basic/Reg1.asm
    Basic/Reg2.asm
    Basic/Reg3.asm
)

set(TestFiles_2
    Exec/Add1.asm
    Exec/Add2.asm
    Exec/Add3.asm
    Exec/Add4.asm
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

add_compile_tests(OutFiles_1 Basic  ${TestFiles_1})
add_compile_tests(OutFiles_2 Exec   ${TestFiles_2})
add_test_dump_err(OutFiles_3 Errors ${TestFiles_3})

set(SRC_ALL
    Catch2.h
    catch/catch.hpp
    Main.cpp
    Parser.cpp
    BlockReader.cpp
    ${OutFiles_1}
    ${OutFiles_2}
    ${OutFiles_3}
    ${ToyVM_BINARY_DIR}/TestConfig.h
)

include_directories(../Source/libtvm ${ToyVM_BINARY_DIR})
add_executable(tvmtest ${SRC_ALL})
target_link_libraries(tvmtest libtvm)

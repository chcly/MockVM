/*
-------------------------------------------------------------------------------
    Copyright (c) 2020 Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include "Declarations.h"

const uint8_t    nullarg[3]  = {0xFF, 0xFF, 0xFF};
const KeywordMap NullKeyword = {{'\0'}, OP_MAX, 0, nullarg};

const uint8_t ArgTypeStd1[3] = {AT_REGI, AT_RVAL, AT_NULL};
const uint8_t ArgTypeStd2[3] = {AT_RVAL, AT_RVAL, AT_NULL};
const uint8_t ArgTypeStd3[3] = {AT_RVAL, AT_NULL, AT_NULL};
const uint8_t ArgTypeStd4[3] = {AT_REGI, AT_RVAL, AT_RVAL};
const uint8_t ArgTypeStd5[3] = {AT_REGI, AT_ADDR, AT_NULL};
const uint8_t ArgTypeStd6[3] = {AT_REGI, AT_RVAL, AT_RVAA};
const uint8_t ArgTypeStd7[3] = {AT_REGI, AT_RIDX, AT_NULL};
const uint8_t ArgTypeStd8[3] = {AT_REGI, AT_SVAL, AT_NULL};
const uint8_t ArgTypeReg1[3] = {AT_REGI, AT_NULL, AT_NULL};
const uint8_t ArgTypeAdr1[3] = {AT_ADDR, AT_NULL, AT_NULL};
const uint8_t ArgTypeNone[3] = {AT_REGI, AT_RVAL, AT_NULL};

const KeywordMap KeywordTable[] = {
    {"mov\0 ", OP_MOV, 2, ArgTypeStd1},
    {"bl\0  ", OP_GTO, 1, ArgTypeAdr1},
    {"ret\0 ", OP_RET, 0, ArgTypeNone},
    {"inc\0 ", OP_INC, 1, ArgTypeReg1},
    {"dec\0 ", OP_DEC, 1, ArgTypeReg1},
    {"cmp\0 ", OP_CMP, 2, ArgTypeStd2},
    {"b\0   ", OP_JMP, 1, ArgTypeAdr1},
    {"beq\0 ", OP_JEQ, 1, ArgTypeAdr1},
    {"bne\0 ", OP_JNE, 1, ArgTypeAdr1},
    {"blt\0 ", OP_JLT, 1, ArgTypeAdr1},
    {"bgt\0 ", OP_JGT, 1, ArgTypeAdr1},
    {"ble\0 ", OP_JLE, 1, ArgTypeAdr1},
    {"bge\0 ", OP_JGE, 1, ArgTypeAdr1},
    {"add\0 ", OP_ADD, 2, ArgTypeStd6},
    {"sub\0 ", OP_SUB, 2, ArgTypeStd4},
    {"mul\0 ", OP_MUL, 2, ArgTypeStd4},
    {"div\0 ", OP_DIV, 2, ArgTypeStd4},
    {"shr\0 ", OP_SHR, 2, ArgTypeStd4},
    {"shl\0 ", OP_SHL, 2, ArgTypeStd4},
    {"adrp\0", OP_ADRP, 2, ArgTypeStd5},
    {"stp\0 ", OP_STP, 2, ArgTypeStd8},
    {"ldp\0 ", OP_LDP, 2, ArgTypeStd8},
    {"str\0 ", OP_STR, 2, ArgTypeStd7},
    {"ldr\0 ", OP_LDR, 2, ArgTypeStd7},
    //  ---- debugging ----
    {"prgi\0", OP_PRI, 0, ArgTypeAdr1},
    {"prg\0 ", OP_PRG, 1, ArgTypeStd3},
    //  ---- debugging ----
};

const size_t KeywordTableSize = sizeof(KeywordTable) / sizeof(KeywordMap);

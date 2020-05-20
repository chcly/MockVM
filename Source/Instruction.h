/*
-------------------------------------------------------------------------------
    Copyright (c) 20120 Charles Carley.

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
#ifndef _Instruction_h_
#define _Instruction_h_

#include <stdint.h>
#include <string.h>

#include "Declarations.h"


enum SectionCodes
{
    SEC_DAT = 0xFF,
    SEC_TXT,
    SEC_STR,
};


enum InstructionFlags
{
    IF_DREG = (1 << 0),
    IF_DLIT = (1 << 1),
    IF_SREG = (1 << 2),
    IF_SLIT = (1 << 3),
    IF_ADDR = (1 << 4),
    IF_MAX  = IF_DREG | IF_DLIT | IF_SREG | IF_SLIT | IF_ADDR
};




struct TVMHeader
{
    uint16_t code;   // 2
    uint8_t  flags;  // 1
    uint8_t  txt;    // 1
    uint32_t dat;    // 4
    uint64_t str;    // 8 -- 16
};

struct TVMSection
{
    uint16_t code;   // 2
    uint16_t flags;  // 2 4
    uint64_t entry;  // 8 12
    uint32_t pad;    // 4 16
    uint32_t size;   // 4 20
    uint32_t start;  // 4 24
    uint64_t padding;// 8 32
};


struct Instruction
{
    uint8_t     op;
    uint8_t     flags;
    uint8_t     argc;
    uint64_t    argv[INS_ARGM];
    uint64_t    label;
    std::string labelName;
};



struct ExecInstruction
{
    uint8_t     op;
    uint8_t     flags;
    uint8_t     argc;
    uint64_t    argv[INS_ARGM];
};


#endif  // _Instruction_h_

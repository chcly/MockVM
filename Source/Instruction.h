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

#define TYPE_ID4(a, b, c, d) ((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))
#define TYPE_ID2(a, b)      ((b) << 8 | (a))


enum SectionCodes
{
    SEC_DAT = 0xFF,
    SEC_TXT,
    SEC_STR,
};


enum InstructionFlags
{
    IF_REG = (1 << 0),
    IF_LIT = (1 << 1),
};


struct Header
{
    uint16_t code;
    uint64_t dat;
    uint64_t str;
    uint64_t txt;
};

struct Section
{
    uint16_t code;
    uint64_t size;
    uint16_t start;
};

struct Instruction
{
    uint8_t  op;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t flags;
};

#endif  // _Instruction_h_

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
#ifndef _Program_h_
#define _Program_h_

#include <stdint.h>
#include <functional>
#include <vector>
#include "Instruction.h"
#include "BlockReader.h"


typedef union Register
{
    uint8_t  a[8];
    uint16_t w[4];
    uint32_t l[2];
    uint64_t x;
} Register;



class Program
{
public:
    typedef std::vector<Instruction> Instructions;
    typedef Register Registers[10];

private:
    Instructions m_ins;
    BlockReader* m_reader;
    Header       m_header;
    Registers    m_regi;

public:
    Program();
    ~Program();

    void load(const char *fname);
    int launch(void);
};

#endif  //_Program_h_

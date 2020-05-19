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
#include <vector>
#include <stack>
#include "Instruction.h"
#include "Decl.h"
#include "BlockReader.h"



#define DeclareOperation(op)\
    int32_t handle##op##(uint8_t op, uint8_t argc, uint8_t flags);

#define DefineOperation(op) \
    int32_t Program::handle##op##(uint8_t op, uint8_t argc, uint8_t flags)


#define OperationTable(op) &Program::handle##op



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
    typedef Register                 Registers[10];

    typedef int32_t (Program::*Operation)(uint8_t op, uint8_t argc, uint8_t flags);
    typedef Operation OpCodes[OP_MAX];

private:
    Instructions m_ins;
    BlockReader* m_reader;
    TVMHeader    m_header;
    Registers    m_regi;
    uint32_t     m_flags;

    std::stack<int32_t> m_stack;

    const static OpCodes OPCodeTable;
    const static size_t  OPCodeTableSize;


    void dumpRegi(void);



    DeclareOperation(OP_RET);
    DeclareOperation(OP_MOV);
    DeclareOperation(OP_INC);
    DeclareOperation(OP_DEC);
    DeclareOperation(OP_CMP);
    DeclareOperation(OP_JMP);
    DeclareOperation(OP_JEQ);
    DeclareOperation(OP_JNE);
    DeclareOperation(OP_JLE);
    DeclareOperation(OP_JGE);
    DeclareOperation(OP_JGT);
    DeclareOperation(OP_JLT);
    DeclareOperation(OP_PRG);

public:
    Program();
    ~Program();

    void load(const char *fname);
    int launch(void);
};

#endif  //_Program_h_

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
#include "Declarations.h"
#include "BlockReader.h"


typedef union Register
{
    uint8_t  a[8];
    uint16_t w[4];
    uint32_t l[2];
    uint64_t x;
} Register;

enum ProgramFlags
{
    PF_E = 1 << 0,
    PF_G = 1 << 1,
    PF_L = 1 << 2,
};


class Program
{
public:
    typedef std::vector<ExecInstruction> Instructions;
    typedef Register                     Registers[10];

    typedef void (Program::*Operation)(ExecInstruction& inst);
    typedef Operation OpCodes[OP_MAX];

private:
    Instructions m_ins;
    BlockReader* m_reader;
    TVMHeader    m_header;
    Registers    m_regi;
    uint32_t     m_flags;
    int32_t      m_return;
    uint64_t     m_curinst;

    std::stack<int32_t> m_stack;

    const static OpCodes OPCodeTable;
    const static size_t  OPCodeTableSize;


    void dumpRegi(void);

    void handle_OP_RET(ExecInstruction& inst);
    void handle_OP_MOV(ExecInstruction& inst);
    void handle_OP_INC(ExecInstruction& inst);
    void handle_OP_DEC(ExecInstruction& inst);
    void handle_OP_CMP(ExecInstruction& inst);
    void handle_OP_JMP(ExecInstruction& inst);
    void handle_OP_JEQ(ExecInstruction& inst);
    void handle_OP_JNE(ExecInstruction& inst);
    void handle_OP_JLE(ExecInstruction& inst);
    void handle_OP_JGE(ExecInstruction& inst);
    void handle_OP_JLT(ExecInstruction& inst);
    void handle_OP_JGT(ExecInstruction& inst);
    void handle_OP_PRG(ExecInstruction& inst);
    void handle_OP_PRGI(ExecInstruction& inst);

public:
    Program();
    ~Program();

    void load(const char *fname);
    int launch(void);
};

#endif  //_Program_h_

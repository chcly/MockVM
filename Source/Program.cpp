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
#include "Program.h"
#include <stdint.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stack>
#include <vector>
#include <string.h>

#include "BlockReader.h"
#include "Declarations.h"
#include "SymbolUtils.h"

using namespace std;

SYM_EXPORT SymbolMapping* std_init();


uint8_t restrict8(const uint8_t& inp,
                  const uint8_t& mi,
                  const uint8_t& ma)
{
    return inp > ma ? ma : inp < mi ? mi : inp;
}

inline bool isRegister(const ExecInstruction& exec, size_t idx)
{
    return idx <= 0 ? (exec.flags & IF_DREG) != 0 : (exec.flags & IF_SREG) != 0;
}



Program::Program() :
    m_flags(0),
    m_return(0),
    m_stdLib(0)
{
    memset(m_regi, 0, sizeof(Registers));
    m_stdLib = std_init();
}

Program::~Program()
{
}

int Program::load(const char* fname)
{
    BlockReader reader = BlockReader(fname);
    if (reader.eof())
    {
        printf("failed to load %s\n", fname);
        return PS_ERROR;
    }

    reader.read(&m_header, sizeof(TVMHeader));
    if (loadStringTable(reader) != PS_OK)
    {
        printf("failed to read string table\n");
        return PS_ERROR;
    }

    if (loadCode(reader) != PS_OK)
    {
        printf("failed to read the text section\n");
        return PS_ERROR;
    }
    return PS_OK;
}


int Program::loadStringTable(BlockReader& reader)
{
    reader.moveTo(m_header.str);
    TVMSection strTab;
    reader.read(&strTab, sizeof(TVMSection));

    if (strTab.size <= 0)
        return PS_OK;

    str_t str;
    uint32_t i;
    size_t   tot =0;
    for (i =0; i<strTab.size; ++i)
    {
        char ch = reader.next();

        if (ch >= 32 && ch <= 127)
            str.push_back(ch);
        else if (ch == 0)
        {
            // if this is correctly stored
            // there should be no duplicates
            m_strtab[str] = tot++;
            str.resize(0);
        }
    }
    return PS_OK;
}

int Program::loadCode(BlockReader& reader)
{
    reader.moveTo(m_header.txt);

    TVMSection code;
    reader.read(&code, sizeof(TVMSection));

    if (code.size <= 0)
        return PS_OK;

    uint8_t  ops[3] = {};
    uint16_t sizes  = 0;
    uint8_t  v8;
    uint16_t v16;
    uint32_t v32;
    int    a;
    size_t i = 0;


    while (i < code.size && !reader.eof())
    {
        i += reader.read(ops, 3);
        i += reader.read(&sizes, 2);

        if (ops[0] >= 0 && ops[0] < OP_MAX)
        {
            ExecInstruction exec = {};
            exec.op    = ops[0];
            exec.argc  = restrict8(ops[1], 0, INS_ARG);
            exec.flags = restrict8(ops[2], 0, IF_MAXF);

            for (a = 0; a < exec.argc; ++a)
            {
                if (sizes & SizeFlags[a][0])
                {
                    i += reader.read(&v8, 1);
                    exec.argv[a] = (uint64_t)v8;
                }
                else if (sizes & SizeFlags[a][1])
                {
                    i += reader.read(&v16, 2);
                    exec.argv[a] = (uint64_t)v16;
                }
                else if (sizes & SizeFlags[a][2])
                {
                    i += reader.read(&v32, 4);
                    exec.argv[a] = (uint64_t)v32;
                }
                else
                {
                    i += reader.read(&exec.argv[a], 8);
                }

                if (exec.flags & IF_SYMA)
                    findStatic(exec);

                if (isRegister(exec, a))
                {
                    // check to see if the register is still valid.
                    if (!(sizes & SizeFlags[a][0]))
                    {
                        printf("error instruction size mismatch\n");
                        return PS_ERROR;
                    }
                }
            }
            m_ins.push_back(exec);
        }
        else
        {
            // assert that the code falls in a valid range.
            printf("error invalid opcode %i\n", ops[0]);
            return PS_ERROR;
        }
    }

    // assert the calculated size with the reported size
    if (i != code.size)
    {
        printf("misaligned instructions\n");
        return PS_ERROR;
    }

    m_curinst = 0;
    if (code.entry < m_ins.size())
        m_curinst = code.entry;

    return PS_OK;
}


void Program::findStatic(ExecInstruction& ins)
{
    int i = 0;
    while (m_stdLib[i].name != 0 && ins.call == nullptr)
    {
        StringMap::iterator it = m_strtab.find(m_stdLib[i].name);
        if (it != m_strtab.end())
        {
            if (ins.argv[0] == it->second)
                ins.call = m_stdLib[i].callback;
        }
        ++i;
    }
}

int Program::launch(void)
{
    if (m_ins.empty())
        return PS_OK; 

    size_t tinst = m_ins.size();

    ExecInstruction* basePtr = m_ins.data();
    m_stack.push(m_curinst);
    while (m_curinst < tinst)
    {
        ExecInstruction& inst = basePtr[m_curinst++];
        uint8_t op = inst.op;
        if (op >= 0 && op < OP_MAX)
        {
            if (OPCodeTable[op] != nullptr)
                (this->*OPCodeTable[op])(inst);
        }
        else
        {
            printf("invalid code");
            return -1;
        }
    }

    return m_return;
}

void Program::handle_OP_RET(ExecInstruction& inst)
{
    if (!m_stack.empty())
    {
        m_curinst = m_stack.top();
        m_stack.pop();
    }

    if (m_stack.empty())
        m_curinst = m_ins.size() + 1;
    m_return = (int32_t)m_regi[0].x;
}

void Program::handle_OP_MOV(ExecInstruction& inst)
{
    if (inst.argv[0] <= 9)
    {
        if (inst.flags & IF_SREG)
        {
            if (inst.argv[1] <= 9)
                m_regi[inst.argv[0]].x = m_regi[inst.argv[1]].x;
        }
        else
            m_regi[inst.argv[0]].x = inst.argv[1];
    }
}

void Program::handle_OP_CALL(ExecInstruction& inst)
{
    if (inst.flags & IF_SYMA)
    {
        if (inst.call != nullptr)
            inst.call(m_regi);
    }
    else
    {
        m_stack.push(m_curinst);
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_INC(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG)
    {
        if (inst.argv[0] <= 9)
            m_regi[inst.argv[0]].x += 1;
    }
}

void Program::handle_OP_DEC(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG)
    {
        if (inst.argv[0] <= 9)
            m_regi[inst.argv[0]].x -= 1;
    }
}

void Program::handle_OP_CMP(ExecInstruction& inst)
{
    uint64_t a = inst.argv[0];
    uint64_t b = inst.argv[1];
    if (inst.flags & IF_DREG && a <= 9)
        a = m_regi[a].x;
    if (inst.flags & IF_SREG && b <= 9)
        b = m_regi[b].x;

    m_flags = 0;
    int64_t r = (int64_t)a - (int64_t)b;
    if (r == 0)
        m_flags |= PF_E;
    else if (r < 0)
        m_flags |= PF_L;
    else if (r > 0)
        m_flags |= PF_G;
}

void Program::handle_OP_JMP(ExecInstruction& inst)
{
    m_curinst = inst.argv[0];
}

void Program::handle_OP_JEQ(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JNE(ExecInstruction& inst)
{
    if (!(m_flags & PF_E))
    {
        m_flags &= ~PF_E;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JLE(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.argv[0];
    }
    else if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JGE(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.argv[0];
    }
    else if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JLT(ExecInstruction& inst)
{
    if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JGT(ExecInstruction& inst)
{
    if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_ADD(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B + C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;

            m_regi[x0].x = b + c;
        }
        else
        {
            // A, B -> A += b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            m_regi[x0].x += b;
        }
    }
}

void Program::handle_OP_SUB(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B - C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;
            m_regi[x0].x = b - c;
        }
        else
        {
            // A, B -> A -= b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            m_regi[x0].x -= b;
        }
    }
}

void Program::handle_OP_MUL(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B * C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;
            m_regi[x0].x = b * c;
        }
        else
        {
            // A, B -> A *= b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            m_regi[x0].x *= b;
        }
    }
}

void Program::handle_OP_DIV(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B / C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;

            if (c != 0)
                m_regi[x0].x = b / c;
        }
        else
        {
            // A, B -> A /= b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (b != 0)
                m_regi[x0].x /= b;
        }
    }
}

void Program::handle_OP_SHR(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B >> C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;

            m_regi[x0].x = b >> c;
        }
        else
        {
            // A, B -> A >>= b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            m_regi[x0].x >>= b;
        }
    }
}

void Program::handle_OP_SHL(ExecInstruction& inst)
{
    const uint64_t& x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        if (inst.argc > 2)
        {
            // A, B, C -> A = B >> C
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            if (c <= 9 && inst.flags & IF_SREG)
                c = m_regi[c].x;

            m_regi[x0].x = b << c;
        }
        else
        {
            // A, B -> A <<= b
            uint64_t b = inst.argv[1];
            if (b <= 9 && inst.flags & IF_SREG)
                b = m_regi[b].x;
            m_regi[x0].x <<= b;
        }
    }
}


void Program::handle_OP_PRG(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG && inst.argv[0] <= 9)
        cout << (int64_t)m_regi[inst.argv[0]].x << '\n';
    else
        cout << (int64_t)inst.argv[0] << '\n';
}

void Program::handle_OP_PRGI(ExecInstruction& inst)
{
    int i;
    for (i = 0; i < 10; ++i)
    {
        cout << setw(4) << ' ' << 'x' << i << ' ';
        cout << setw(22);
        cout << m_regi[i].x << setw(4) << ' ';
        cout << setfill('0');
        cout << hex;
        cout << setw(2) << (int)m_regi[i].b[7] << ' ';
        cout << setw(2) << (int)m_regi[i].b[6] << ' ';
        cout << setw(2) << (int)m_regi[i].b[5] << ' ';
        cout << setw(2) << (int)m_regi[i].b[4] << ' ';
        cout << setw(2) << (int)m_regi[i].b[3] << ' ';
        cout << setw(2) << (int)m_regi[i].b[2] << ' ';
        cout << setw(2) << (int)m_regi[i].b[1] << ' ';
        cout << setw(2) << (int)m_regi[i].b[0] << ' ';
        cout << dec;
        cout << setfill(' ');
        cout << '\n';
    }
}

const Program::Operation Program::OPCodeTable[] = {
    nullptr,
    &Program::handle_OP_RET,
    &Program::handle_OP_MOV,
    &Program::handle_OP_CALL,
    &Program::handle_OP_INC,
    &Program::handle_OP_DEC,
    &Program::handle_OP_CMP,
    &Program::handle_OP_JMP,
    &Program::handle_OP_JEQ,
    &Program::handle_OP_JNE,
    &Program::handle_OP_JLT,
    &Program::handle_OP_JGT,
    &Program::handle_OP_JLE,
    &Program::handle_OP_JGE,
    &Program::handle_OP_ADD,
    &Program::handle_OP_SUB,
    &Program::handle_OP_MUL,
    &Program::handle_OP_DIV,
    &Program::handle_OP_SHR,
    &Program::handle_OP_SHL,
    &Program::handle_OP_PRG,
    &Program::handle_OP_PRGI,
};

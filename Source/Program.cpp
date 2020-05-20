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
#include "BlockReader.h"
#include "Declarations.h"

using namespace std;

uint8_t restrict8(const uint8_t& inp,
                  const uint8_t& mi,
                  const uint8_t& ma)
{
    return inp > ma ? ma : inp < mi ? mi : inp;
}

Program::Program() :
    m_reader(0),
    m_flags(0),
    m_return(0)
{
    memset(m_regi, 0, sizeof(Registers));
}

Program::~Program()
{
    delete m_reader;
}

void Program::load(const char* fname)
{
    if (m_reader)
        delete m_reader;

    m_reader = new BlockReader(fname);
    if (!m_reader->eof())
        m_reader->read(&m_header, sizeof(TVMHeader));

    m_reader->moveTo(m_header.txt);
    TVMSection code;
    m_reader->read(&code, sizeof(TVMSection));

    uint8_t ops[4];
    size_t  i = 0;
    while (i < code.size)
    {
        if (m_reader->eof())
            break;

        m_reader->read(ops, 4);
        i += 4;
        if (ops[0] >= 0 && ops[0] < OP_MAX)
        {
            ExecInstruction exec = {};
            exec.op    = restrict8(ops[0], OP_RET, OP_MAX - 1);
            exec.argc  = restrict8(ops[1], 0, 3);
            exec.flags = restrict8(ops[2], 0, IF_MAX);

            if (exec.argc > 0)
                m_reader->read(&exec.argv[0], 8);
            if (exec.argc > 1)
                m_reader->read(&exec.argv[1], 8);
            if (exec.argc > 2)
                m_reader->read(&exec.argv[2], 8);

            i += (8 * (size_t)exec.argc);

            m_ins.push_back(exec);
        }
    }

    m_curinst = 0;
    if (code.entry < m_ins.size())
        m_curinst = code.entry;
}

int Program::launch(void)
{
    if (!m_reader)
        return -1;

    size_t           tinst   = m_ins.size();
    ExecInstruction* basePtr = m_ins.data();
    m_stack.push(m_curinst);

    while (m_curinst < tinst)
    {
        ExecInstruction& inst = basePtr[m_curinst++];
        if (OPCodeTable[inst.op] != nullptr)
            (this->*OPCodeTable[inst.op])(inst);
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
    m_stack.push(m_curinst);
    m_curinst = inst.argv[0];
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
    if (a == b)
        m_flags |= PF_E;
    else if (a < b)
        m_flags |= PF_L;
    else if (a > b)
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
    uint64_t x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        uint64_t a = m_regi[x0].x;
        uint64_t b = inst.argv[1];

        if (inst.argv[1] <= 9 && inst.flags & IF_SREG)
            b = m_regi[b].x;

        m_regi[x0].x = a + b;
    }
}

void Program::handle_OP_SUB(ExecInstruction& inst)
{
    uint64_t x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        uint64_t a = m_regi[x0].x;
        uint64_t b = inst.argv[1];

        if (inst.argv[1] <= 9 && inst.flags & IF_SREG)
            b = m_regi[b].x;

        m_regi[x0].x = a - b;
    }
}

void Program::handle_OP_MUL(ExecInstruction& inst)
{
    uint64_t x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        uint64_t a = m_regi[x0].x;
        uint64_t b = inst.argv[1];

        if (inst.argv[1] <= 9 && inst.flags & IF_SREG)
            b = m_regi[b].x;

        m_regi[x0].x = a * b;
    }
}

void Program::handle_OP_DIV(ExecInstruction& inst)
{
    uint64_t x0 = inst.argv[0];
    if (x0 <= 9 && inst.flags & IF_DREG)
    {
        uint64_t a = m_regi[x0].x;
        uint64_t b = inst.argv[1];

        if (inst.argv[1] <= 9 && inst.flags & IF_SREG)
            b = m_regi[b].x;

        if (b != 0)
            m_regi[x0].x = a / b;
    }
}

void Program::handle_OP_PRG(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG && inst.argv[0] <= 9)
        cout << m_regi[inst.argv[0]].x << '\n';
    else
        cout << inst.argv[0] << '\n';
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
    &Program::handle_OP_PRG,
    &Program::handle_OP_PRGI,
};

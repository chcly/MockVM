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
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <functional>
#include <vector>
#include <stack>
#include "Instruction.h"
#include "Program.h"
#include "BlockReader.h"
#include "Decl.h"

using namespace std;

uint8_t restrict8(const uint8_t &inp, const uint8_t &mi, const uint8_t& ma)
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

void Program::load(const char *fname)
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
        if (ops[0]>=0 && ops[0] < OP_MAX)
        {
            ExecInstruction exec = {};

            exec.op    = restrict8(ops[0], OP_RET, OP_MAX - 1);
            exec.argc  = restrict8(ops[1], 0, 3);
            exec.flags = restrict8(ops[2], 0, IF_MAX);

            if (exec.argc > 0)
                m_reader->read(&exec.arg1, 8);
            if (exec.argc > 1)
                m_reader->read(&exec.arg2, 8);
            if (exec.argc > 2)
                m_reader->read(&exec.arg3, 8);

            i += (8 * (size_t)exec.argc);

            m_ins.push_back(exec);
        }
    }
}


int Program::launch(void)
{
    if (!m_reader)
        return -1;

    int32_t tinst = m_ins.size();

    ExecInstruction *basePtr = m_ins.data();

    m_stack.push(0);
    m_curinst = 0;
    while (m_curinst < tinst)
    {
        ExecInstruction &inst = basePtr[m_curinst++];
        if (OPCodeTable[inst.op] != nullptr)
            (this->*OPCodeTable[inst.op])(inst);
    }
    return m_return;
}


void Program::handle_OP_RET(ExecInstruction& inst)
{
    m_stack.pop();
    if (m_stack.empty())
        m_curinst = m_ins.size() + 1;
    m_return = (int32_t)m_regi[0].x;
}

void Program::handle_OP_MOV(ExecInstruction& inst)
{
    if (inst.arg1 <= 9)
    {
        if (inst.flags & IF_SREG)
        {
            if (inst.arg2 <= 9)
                m_regi[inst.arg1].x = m_regi[inst.arg2].x;
        }
        else
            m_regi[inst.arg1].x = inst.arg2;
    }
}

void Program::handle_OP_INC(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG)
    {
        if (inst.arg1 <= 9)
            m_regi[inst.arg1].x += 1; 
    }
}


void Program::handle_OP_DEC(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG)
    {
        if (inst.arg1 <= 9)
            m_regi[inst.arg1].x -= 1;
    }
}

void Program::handle_OP_CMP(ExecInstruction& inst)
{
    uint64_t a, b;
    a = inst.arg1;
    b = inst.arg2;

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
    m_curinst = inst.arg1;
}

void Program::handle_OP_JEQ(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.arg1;
    }
}

void Program::handle_OP_JNE(ExecInstruction& inst)
{
    if (!(m_flags & PF_E))
    {
        m_flags &= ~PF_E;
        m_curinst = inst.arg1;
    }
}


void Program::handle_OP_JLE(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.arg1;
    }
    else if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.arg1;
    }
}


void Program::handle_OP_JGE(ExecInstruction& inst)
{
    if (m_flags & PF_E)
    {
        m_flags &= ~PF_E;
        m_curinst = inst.arg1;
    }
    else if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.arg1;
    }
}


void Program::handle_OP_JLT(ExecInstruction& inst)
{
    if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.arg1;
    }
}

void Program::handle_OP_JGT(ExecInstruction& inst)
{
    if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.arg1;
    }
}


void Program::handle_OP_PRG(ExecInstruction& inst)
{
    if (inst.flags & IF_DREG && inst.arg1 <= 9)
        cout << m_regi[inst.arg1].x << '\n';
    else
        cout << inst.arg1 << '\n';
}

void Program::handle_OP_PRGI(ExecInstruction& inst)
{
    for (int i = 0; i < 10; ++i)
    {
        if (m_regi[i].x != 0)
        {
            cout << setw(4) << ' ' << 'x' << i << ' ';
            cout << setw(22);
            cout << m_regi[i].x << setw(4) << ' ';
            cout << setfill('0');
            cout << hex;
            cout << setw(2) << (int)m_regi[i].a[7] << ' ';
            cout << setw(2) << (int)m_regi[i].a[6] << ' ';
            cout << setw(2) << (int)m_regi[i].a[5] << ' ';
            cout << setw(2) << (int)m_regi[i].a[4] << ' ';
            cout << setw(2) << (int)m_regi[i].a[3] << ' ';
            cout << setw(2) << (int)m_regi[i].a[2] << ' ';
            cout << setw(2) << (int)m_regi[i].a[1] << ' ';
            cout << setw(2) << (int)m_regi[i].a[0] << ' ';
            cout << dec;
            cout << setfill(' ');
            cout << '\n';
        }
    }
}


const Program::Operation Program::OPCodeTable[] = {
    nullptr,
    &Program::handle_OP_RET,
    &Program::handle_OP_MOV,
    nullptr,  //OperationTable(OP_CALL),
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
    &Program::handle_OP_PRG,
    &Program::handle_OP_PRGI,
};

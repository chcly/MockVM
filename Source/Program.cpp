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


Program::Program() :
    m_reader(0),
    m_flags(0)
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
}


int Program::launch(void)
{
    if (!m_reader)
        return -1;


    m_reader->moveTo(m_header.txt + sizeof(TVMSection));
    
    int32_t rc = 0;
    uint8_t ops[4];

    m_stack.push(rc);

    while (!m_stack.empty())
    {
        if (m_reader->eof())
            break;


        m_reader->read(ops, 4);
        if (ops[0] >=0 && ops[0] < OP_MAX)
        {
            if (OPCodeTable[ops[0]] != nullptr)
                rc = (this->*OPCodeTable[ops[0]])(ops[0], ops[1], ops[2]);
        }
    }
    return rc;
}



DefineOperation(OP_RET)
{
    if (!m_stack.empty())
        m_stack.pop();
    return m_regi[0].x;
}


DefineOperation(OP_MOV)
{
    uint64_t reg, dst;
    m_reader->read(&reg, 8);
    m_reader->read(&dst, 8);

    if (reg <= 9)
    {
        Register &r = m_regi[reg];
        if (flags & IF_SREG)
        {
            if (dst <= 9)
                r.x = m_regi[dst].x;
        }
        else
            r.x = dst;
    }
    return 0;
}

DefineOperation(OP_INC) 
{
    uint64_t reg;
    m_reader->read(&reg, 8);
    if (reg <= 9)
    {
        Register &r = m_regi[reg];
        r.x += 1;
    }
    return 0;
}


DefineOperation(OP_DEC)
{
    uint64_t reg;
    m_reader->read(&reg, 8);
    if (reg <= 9)
    {
        Register &r = m_regi[reg];
        r.x -= 1;
    }
    return 0;
}

DefineOperation(OP_CMP)
{
    uint64_t a, b;
    m_reader->read(&a, 8);
    m_reader->read(&b, 8);

    if (flags & IF_DREG && a <= 9)
        a = m_regi[a].x;
    if (flags & IF_SREG && b <= 9)
        b = m_regi[b].x;

    if (a - b == 0)
        m_flags |= 1;
    return 0;
}

DefineOperation(OP_JMP)
{
    uint64_t a;
    m_reader->read(&a, 8);
    m_reader->moveTo(a);
    return 0;
}


DefineOperation(OP_JEQ)
{
    if (m_flags & 1)
    {
        m_flags &= ~1;
        uint64_t a;
        m_reader->read(&a, 8);
        m_reader->moveTo(a);
    }
    return 0;
}


DefineOperation(OP_JNE)
{
    if (!(m_flags & 1))
    {
        m_flags &= ~1;
        uint64_t a;
        m_reader->read(&a, 8);
        m_reader->moveTo(a);
    }
    return 0;
}


DefineOperation(OP_JLT)
{
    return 0;
}


DefineOperation(OP_JGT)
{
    return 0;
}


DefineOperation(OP_JLE)
{
    return 0;
}


DefineOperation(OP_JGE)
{
    return 0;
}


DefineOperation(OP_PRG)
{
    uint64_t a;
    m_reader->read(&a, 8);
    if (flags & IF_DREG && a <= 9)
        a = m_regi[a].x;
    cout << a << '\n';
    return 0;
}



void Program::dumpRegi(void)
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
    OperationTable(OP_RET),
    OperationTable(OP_MOV),
    nullptr,  //OperationTable(OP_CALL),
    OperationTable(OP_INC),
    OperationTable(OP_DEC),
    OperationTable(OP_CMP),
    OperationTable(OP_JMP),
    OperationTable(OP_JEQ),
    OperationTable(OP_JNE),
    OperationTable(OP_JLT),
    OperationTable(OP_JGT),
    OperationTable(OP_JLE),
    OperationTable(OP_JGE),
    OperationTable(OP_PRG),
    nullptr,  // OP_TRACE
};
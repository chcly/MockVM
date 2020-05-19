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
    m_reader(0)
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
        m_reader->read(&m_header, sizeof(Header));
}


int Program::launch(void)
{
    if (!m_reader)
        return -1;


    m_reader->moveto(m_header.txt + sizeof(Section));

    std::stack<int> m_stack;

    int rc = 0;
    m_stack.push(rc);

    while (!m_stack.empty())
    {
        if (m_reader->eof())
            break;

        uint8_t ops[4];
        m_reader->read(ops, 4);

        switch (ops[0])
        {
        case OP_MOV:
        {
            uint64_t reg, dst;
            m_reader->read(&reg, 8);
            m_reader->read(&dst, 8);

            if (reg <= 9)
            {
                Register &r = m_regi[reg];

                if (ops[2] & IF_SREG)
                {
                    if (dst <= 9)
                        r.x = m_regi[dst].x;
                }
                else
                    r.x = dst;
            }
        } break;
        case OP_TRACE:
            dumpRegi();
            break;
        case OP_RET:
            rc = m_regi[0].x;
            m_stack.pop();
            break;
        }
    }
    return rc;
}


void Program::dumpRegi(void)
{
    for (int i=0; i<10; ++i)
        cout << setfill(' ') << setw(4) << ' ' << 'x' << i << ' '
              << "0x"<<  setw(16) << setfill('0') << hex << m_regi[i].x << '\n';
}

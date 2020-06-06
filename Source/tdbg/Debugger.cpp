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
#include "Debugger.h"
#include <iomanip>
#include <sstream>
#include "Console.h"
#include "SymbolUtils.h"

using namespace std;

#define COL1_ST  2
#define COL2_ST  41
#define COL1_LAB 10
#define COL2_LAB 55
#define MAX_RIGHT 80
#define MIN_Y     0

const ExecInstruction nop = {0, 0, 0, {0, 0, 0}, 0, 0};

Debugger::Debugger(const str_t& mod, const str_t& file) :
    Program(mod),
    m_exit(false),
    m_file(file),
    m_console(nullptr)
{
    memset(m_last, 0, sizeof(Registers));
    m_ypos    = 0;
    m_console = GetPlatformConsole();
}

Debugger::~Debugger()
{
    delete m_console;
}

void Debugger::displayHeader(void)
{
    m_ypos = MIN_Y;
    m_console->setColor(CS_GREY);
    m_console->displayLineHorz(0, MAX_RIGHT, m_ypos);
    m_console->displayLineHorz(0, MAX_RIGHT, 27);
    m_console->displayLineVert(m_ypos + 1, 27, COL2_ST - 2);
    m_console->displayLineHorz(COL2_ST - 1, MAX_RIGHT, m_ypos + MAX_REG + 1);
    m_console->displayLineHorz(COL2_ST - 1, MAX_RIGHT, m_ypos + MAX_REG + 7);
    m_console->setColor(CS_GREEN);
    m_console->displayString("Instructions", COL1_LAB, m_ypos);
    m_console->displayString("Registers", COL2_LAB, m_ypos);
    m_console->displayString("Stack", COL2_LAB, m_ypos + MAX_REG + 1);
    m_console->setColor(CS_WHITE);
    ++m_ypos;
}

int Debugger::debug(void)
{
    if (!m_console)
        return -1;

    m_callStack.push(m_curinst);

    render();
    while (!m_exit)
    {
        int cmd = (int)m_console->getNextCmd();
        if (cmd == 27)
            m_exit = true;

        if (cmd == 13)
        {
            render();
            step();
        }
    }
    return 0;
}

void Debugger::render(void)
{
    m_ypos = MIN_Y;
    m_console->clear();
    displayHeader();
    displayInstructions();
    m_console->flush();
}

void Debugger::displayInstructions(void)
{
    int16_t start = (int16_t)m_curinst, i;
    int16_t ma    = 25;
    int16_t hs    = ma >> 1;

    for (i = 0; i < hs && start > 1; ++i)
        start--;

    for (i = 0; i < ma; ++i)
    {
        size_t cinst = start + i;
        if (cinst < m_ins.size())
            disassemble(m_ins.at(cinst), cinst);
        else
            disassemble(nop, cinst);
    }
    displayRegisters();
    displayStack();
}

void Debugger::step(void)
{
    if (m_curinst < m_ins.size())
    {
        const ExecInstruction& inst = m_ins.at((size_t)m_curinst++);
        if (OPCodeTable[inst.op] != nullptr)
            (this->*OPCodeTable[inst.op])(inst);
    }
    else
        m_exit = true;
}

void Debugger::displayRegisters(void)
{
    stringstream ss, ss1;
    int          i;
    int          line = MIN_Y + 1;

    m_console->setColor(CS_WHITE);

    for (i = 0; i < MAX_REG; ++i, ++line)
    {
        ss << "0x" << setfill('0') << uppercase << hex << m_regi[i].x;
        ss1 << ' ' << 'x' << i << ' ';
        ss1 << right << setw(17) << ss.str();
        ss1 << ' ';
        ss1 << dec;
        ss1 << setw(17);
        ss1 << m_regi[i].x;

        if (m_regi[i].x != m_last[i].x)
            m_console->setColor(CS_RED);
        else
            m_console->setColor(CS_LIGHT_GREY);

        m_last[i].x = m_regi[i].x;

        m_console->displayString(ss1.str(), COL2_ST - 1, line);
        ss.str("");
        ss1.str("");
    }
}

void Debugger::displayStack(void)
{
    int line = MIN_Y + MAX_REG + 2;
    if (m_stack.empty())
        return;
    m_console->setColor(CS_WHITE);

    line++;
    uint32_t stk = m_stack.size(), i;
    if (stk > 4)
        stk = 4;

    for (i = 0; i < stk; ++i)
    {
        stringstream ss;
        ss << right << setw(2) << i << ' ';
        ss << right << setw(16) << m_stack.peek(i);
        m_console->displayString(ss.str(), COL2_ST, line++);
    }
}

void Debugger::disassemble(const ExecInstruction& inst, size_t i)
{
    ostringstream ss;
    ss << right << setw(4) << setfill('0') << uppercase << hex << i;
    ss << ' ' << setfill(' ');

    if (i == m_curinst)
    {
        ss << "==> ";
        m_console->setColor(CS_YELLOW);
    }
    else
    {
        ss << setw(4) << ' ';
        m_console->setColor(CS_LIGHT_GREY);
    }

    str_t op;
    getOpString(op, inst.op);
    ss << left << setw(6) << op;

    if (inst.argc > 0)
    {
        if (inst.flags & IF_SYMU)
            ss << "0x" << (size_t)inst.call;
        else if (inst.flags & IF_STKP)
            ss << "sp";
        else if (inst.flags & IF_INSP)
            ss << "sp";
        else if (inst.flags & IF_REG0)
            ss << 'x' << inst.argv[0];
        else
            ss << "0x" << inst.argv[0];
    }

    if (inst.argc > 1)
    {
        ss << ", ";
        if (inst.flags & IF_REG1)
            ss << 'x' << inst.argv[1];
        else
            ss << inst.argv[1];
    }

    if (inst.argc > 2)
    {
        ss << ", ";
        if (inst.flags & IF_REG2)
            ss << 'x' << inst.argv[2];
        else
            ss << inst.argv[2];
    }

    m_console->displayString(ss.str(), COL1_ST, m_ypos++);
}

void Debugger::getOpString(str_t& dest, const uint8_t op)
{
    switch (op)
    {
    case OP_RET:
        dest = "ret";
        break;
    case OP_MOV:
        dest = "mov";
        break;
    case OP_GTO:
        dest = "bl";
        break;
    case OP_INC:
        dest = "inc";
        break;
    case OP_DEC:
        dest = "inc";
        break;
    case OP_CMP:
        dest = "cmp";
        break;
    case OP_JMP:
        dest = "b";
        break;
    case OP_JEQ:
        dest = "beq";
        break;
    case OP_JNE:
        dest = "bne";
        break;
    case OP_JLT:
        dest = "blt";
        break;
    case OP_JGT:
        dest = "bgt";
        break;
    case OP_JLE:
        dest = "ble";
        break;
    case OP_JGE:
        dest = "bge";
        break;
    case OP_ADD:
        dest = "add";
        break;
    case OP_SUB:
        dest = "sub";
        break;
    case OP_MUL:
        dest = "mul";
        break;
    case OP_DIV:
        dest = "div";
        break;
    case OP_SHR:
        dest = "shr";
        break;
    case OP_SHL:
        dest = "shl";
        break;
    case OP_ADRP:
        dest = "adrp";
        break;
    case OP_STR:
        dest = "str";
        break;
    case OP_LDR:
        dest = "ldr";
        break;
    case OP_LDRS:
        dest = "ldrs";
        break;
    case OP_STRS:
        dest = "strs";
        break;
    case OP_STP:
        dest = "stp";
        break;
    case OP_LDP:
        dest = "ldp";
        break;
    case OP_PRG:
        dest = "prg";
        break;
    case OP_PRI:
        dest = "prgi";
        break;
    default:
        dest = "nop";
        break;
    }
}

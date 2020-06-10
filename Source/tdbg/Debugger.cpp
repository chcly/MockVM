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
#include <iostream>
#include <sstream>
#include "Console.h"
#include "SymbolUtils.h"

using namespace std;

const ExecInstruction nop = {0, 0, 0, {0, 0, 0}, 0, 0};

Debugger::Debugger(const str_t& mod, const str_t& file) :
    Program(mod),
    m_exit(false),
    m_file(file),
    m_console(nullptr),
    m_last(),
    m_lastAddr(-1),
    m_baseAddr(0)
{
    initialize();
}

Debugger::~Debugger()
{
    delete m_console;
}

void Debugger::initialize()
{
    memset(m_last, 0, sizeof(Registers));
    m_console = GetPlatformConsole();
    if (!m_console)
    {
        printf("Console initialization failed\n");
        return;
    }

    const ConsoleRect& rect = m_console->getRect();

    m_instRect.x = 0;
    m_instRect.y = 0;
    m_instRect.w = 30;
    m_instRect.h = rect.h;

    m_regiRect.x = m_instRect.right() + 1;
    m_regiRect.y = 0;
    m_regiRect.w = 43;
    m_regiRect.h = 11;

    m_stackRect.x = m_instRect.right() + 1;
    m_stackRect.y = m_regiRect.bottom() + 1;
    m_stackRect.w = m_regiRect.w;
    m_stackRect.h = 5;

    m_dataRect.x = m_instRect.right() + 1;
    m_dataRect.y = m_stackRect.bottom() + 1;
    m_dataRect.w = m_regiRect.w;
    m_dataRect.h = rect.h;

    m_outRect.x = m_regiRect.right() + 1;
    m_outRect.y = 0;
    m_outRect.w = rect.w - (m_instRect.w + m_regiRect.w);
    m_outRect.h = rect.h;

    m_console->setOutputRect({m_outRect.x, int16_t(m_outRect.y + 1), m_outRect.w, m_outRect.h});
}

void Debugger::displayHeader(void)
{
    str_t              string;
    std::ostringstream stream;

    //m_console->setColor(CS_GREY);
    //m_console->displayLineHorz(m_instRect.x, m_instRect.right(), m_instRect.y);
    //m_console->displayLineHorz(m_regiRect.x, m_regiRect.right(), m_regiRect.y);
    //m_console->displayChar('+', m_regiRect.x-1, m_regiRect.y);
    //m_console->displayLineHorz(m_stackRect.x, m_stackRect.right(), m_stackRect.y);
    //m_console->displayLineHorz(m_dataRect.x, m_dataRect.right(), m_dataRect.y);
    //m_console->displayLineHorz(m_outRect.x, m_outRect.right()-1, m_outRect.y);
    //m_console->displayChar('+', m_outRect.x-1, m_outRect.y);
    //m_console->displayLineVert(m_instRect.y+1, m_instRect.bottom(), m_instRect.right());
    //m_console->displayLineVert(m_regiRect.y + 1, m_instRect.bottom(), m_regiRect.right());

    m_console->setColor(CS_GREEN);

    stream << "Instructions [" << m_ins.size() << "]";
    string = stream.str();

    m_console->displayString(string,
                             m_instRect.cx() - (int16_t)(string.size() >> 1),
                             m_instRect.y);
    m_console->displayString("Registers",
                             m_regiRect.cx() - 4,
                             m_regiRect.y);

    stream.str("");
    stream << "Stack [" << m_stack.size() << "]";
    string = stream.str();

    m_console->displayString(string,
                             m_stackRect.cx() - (int16_t)(string.size() >> 1),
                             m_stackRect.y);

    m_console->displayString("Data",
                             m_dataRect.cx() - 2,
                             m_dataRect.y);

    m_console->displayString("Output",
                             m_outRect.cx() - 2,
                             m_outRect.y);

    m_console->setColor(CS_WHITE);
}

int Debugger::debug(void)
{
    if (!m_console)
        return -1;

    // save the original data table
    m_dataTable.cloneInto(m_dataTableCpy);

top:
    m_callStack.push(m_curinst);

    render();
    while (!m_exit)
    {
        int cmd = m_console->getNextCmd();
        if (cmd == CCS_QUIT)
            m_exit = true;
        if (cmd == CCS_STEP)
        {
            render();
            step();
        }
    }

    displayExit();
    if (m_console->getNextCmd() == CCS_RESTART)
        goto top;
    return 0;
}

void Debugger::render(void)
{
    m_console->clear();
    displayHeader();
    displayInstructions();
    displayRegisters();
    displayStack();
    displayData();
    displayOutput();
    m_console->flush();
}

void Debugger::displayInstructions(void)
{
    m_ypos = m_instRect.y + 1;

    int16_t start = (int16_t)m_curinst, i;
    int16_t ma    = m_instRect.h - 1;
    int16_t hs    = ma - 5;
    start -= hs;
    if (start < 0)
        start = 0;

    for (i = 0; i < ma; ++i)
    {
        size_t cinst = (size_t)start + (size_t)i;
        if (cinst < m_ins.size())
            disassemble(m_ins.at(cinst), cinst);
        else
            disassemble(nop, cinst);
    }
}

void Debugger::step(void)
{
    if (m_curinst < m_ins.size())
    {
        const ExecInstruction& inst = m_ins.at((size_t)m_curinst++);
        if (OPCodeTable[inst.op] != nullptr)
        {
            if ((inst.op == OP_GTO && inst.call) || (inst.op >= OP_PRG))
                m_console->switchOutput(true);

            (this->*OPCodeTable[inst.op])(inst);

            if ((inst.op == OP_GTO && inst.call) || (inst.op >= OP_PRG))
                m_console->switchOutput(false);
        }
    }
    else
        m_exit = true;
}

void Debugger::displayRegisters(void)
{
    stringstream ss, ss1;
    int          i;
    int          line = m_regiRect.y + 1;

    m_console->setColor(CS_WHITE);

    for (i = 0; i < MAX_REG; ++i, ++line)
    {
        ss << "0x" << setfill('0') << uppercase << hex << m_regi[i].x;
        ss1 << 'x' << i << ' ';
        ss1 << right << setw(19) << ss.str();
        ss1 << ' ';
        ss1 << dec;
        ss1 << setw(19);
        ss1 << m_regi[i].x;

        if (m_regi[i].x != m_last[i].x)
            m_console->setColor(CS_RED);
        else
            m_console->setColor(CS_LIGHT_GREY);

        m_last[i].x = m_regi[i].x;
        m_console->displayString(ss1.str(), m_regiRect.x, line);

        ss.str("");
        ss1.str("");
    }

    ss << "flags: [";
    if (m_flags & PF_Z)
        ss << ' ' << 'Z';
    if (m_flags & PF_G)
        ss << ' ' << 'G';
    if (m_flags & PF_L)
        ss << ' ' << 'L';
    ss << ' ' << ']';

    m_console->setColor(CS_DARKCYAN);
    m_console->displayString(ss.str(), m_regiRect.x, line);
}

void Debugger::displayStack(void)
{
    int16_t line = m_stackRect.y + 1;
    if (m_stack.empty())
        return;

    m_console->setColor(CS_LIGHT_GREY);
    uint32_t stk = (uint32_t)m_stack.size(), i;
    if (stk > 4)
        stk = 4;

    for (i = 0; i < stk; ++i, ++line)
    {
        m_console->displayChar('I', m_stackRect.x, line);
        m_console->displayInt(i, m_stackRect.x + 2, line);
        m_console->displayIntRightAligned((int)m_stack.peek(i),
                                          m_stackRect.right() - 1,
                                          line);
    }
}

void Debugger::displayData(void)
{
    uint8_t* data = m_dataTable.ptr();
    if (!data || (m_dataTable.capacity() <= 0) || (m_curinst >= m_ins.size()))
        return;
    if (m_baseAddr == 0)
        m_baseAddr = (size_t)data;

    int line = m_dataRect.y + 1;

    const ExecInstruction& inst = m_ins.at((size_t)m_curinst);
    switch (inst.op)
    {
    case OP_STRS:
    case OP_LDRS:

        if (inst.flags & IF_REG1)
        {
            const Register& src = m_regi[inst.argv[1]];
            m_baseAddr          = (size_t)src.x;
            if (m_baseAddr)
            {
                m_lastAddr = 0;
                if (inst.index < MAX_REG)
                {
                    size_t i = (size_t)m_regi[inst.index].x;
                    if (i < m_dataTable.capacity())
                        m_lastAddr = i;
                }
                else
                    m_lastAddr = inst.index;
            }
        }
        break;
    default:
        break;
    }

    m_console->setColor(CS_LIGHT_GREY);
    size_t sz = m_dataTable.capacity(), i;

    if (m_baseAddr != 0 && m_baseAddr != -1)
    {
        int16_t x, y;
        x = m_dataRect.x;
        y = m_dataRect.y + 1;

        uint8_t* st = (uint8_t*)m_baseAddr;
        uint8_t* en = data + m_dataTable.capacity();

        size_t sz = en - st;

        for (i = 0; i < sz && y < m_dataRect.bottom(); ++i)
        {
            uint8_t ch = (*st++);

            if (i == m_lastAddr)
                m_console->setColor(CS_WHITE, CS_RED);
            else if (ch == 0)
                m_console->setColor(CS_GREY);
            else
                m_console->setColor(CS_LIGHT_GREY);

            m_console->displayCharHex(ch, x++, y);
            x += 2;

            if (x + 2 > m_dataRect.right())
            {
                x = m_dataRect.x;
                y++;
            }
        }
    }
}

void Debugger::displayOutput(void)
{
    m_console->setColor(CS_DARKCYAN);
    m_console->displayOutput(m_outRect.x, m_outRect.y + 1);
}

void Debugger::displayExit(void)
{
    if (m_curinst > 0)
        --m_curinst;

    std::ostringstream ss;
    ss << "\nExited with code " << m_return << '\n';

    m_console->appendOutput(ss.str());

    ss.str("");
    ss << "Press (q) to exit or (r) to restart.\n\n";
    m_console->appendOutput(ss.str());
    m_console->getOutputLineCount();

    render();

    m_console->flush();
    m_console->pause();

    // reset all state variables
    m_console->clearOutput();

    m_stack.clear();
    memset(m_regi, 0, sizeof(Registers));
    memset(m_last, 0, sizeof(Registers));

    m_baseAddr = 0;
    m_lastAddr = -1;
    m_dataTableCpy.cloneInto(m_dataTable);

    m_curinst = m_startinst;
    m_exit    = false;
}

void Debugger::disassemble(const ExecInstruction& inst, size_t i)
{
    ostringstream ss;
    ss << right << setw(4) << setfill('0') << uppercase << hex << i;
    ss << ' ' << setfill(' ');

    if (i == m_curinst)
    {
        ss << "==>";
        m_console->setColor(CS_YELLOW, CS_DARKMAGENTA);
    }
    else
    {
        ss << setw(3) << ' ';
        m_console->setColor(CS_WHITE);
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
        if (inst.flags & IF_RIDX)
        {
            ss << '[';
            ss << 'x' << inst.argv[1];
            ss << ", ";

            if (inst.flags & IF_REG1)
                ss << 'x' << inst.index;
            else
                ss << dec << inst.index << hex;
            ss << ']';
        }
        else if (inst.flags & IF_STKP)
        {
            ss << dec << inst.argv[1];
            ss << hex;
        }
        else if (inst.flags & IF_REG1)
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

    m_console->displayString(ss.str(), m_instRect.x, m_ypos++);
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

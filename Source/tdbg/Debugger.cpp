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
#include "InstructionWriter.h"
#include "SymbolUtils.h"

using namespace std;

const DebugInstruction nop = {0, "nop", {0, 0, 0, {0, 0, 0}, 0, 0}};

Debugger::Debugger(const str_t& mod, const str_t& file) :
    Program(mod),
    m_file(file),
    m_console(nullptr),
    m_last(),
    m_lastAddr(-1),
    m_baseAddr(0),
    m_maxInstWidth(0)
{
    initialize();
}

Debugger::~Debugger()
{
    delete m_console;
}

void Debugger::initialize()
{
    m_console = GetPlatformConsole();

    if (!m_console)
    {
        printf("Console initialization failed\n");
        return;
    }

    memset(m_last, 0, sizeof(Registers));
}

int Debugger::debug(void)
{
    if (!m_console)
        return -1;

    constructDebugInfo();
    calculateDisplayRects();

    m_dataTable.cloneInto(m_dataTableCpy);

    int cmd = CCS_NO_INPUT;

top:

    m_callStack.push(m_curinst);
    render();

    while (!m_exit)
    {
        cmd = m_console->nextCommand();

        switch (cmd)
        {
        case CCS_FORCE_EXIT:
        case CCS_QUIT:
            m_exit = true;
            break;
        case CCS_ADD_BREAKPOINT:
            addBreakPoint();
            render();
            break;
        case CCS_CONTINUE:
            stepToNextBreakPoint();
            render();
            break;
        case CCS_STEP:
            stepOneInstruction();
            render();
            break;
        case CCS_STEP_OUT:
            stepOut();
            render();
            break;
        case CCS_RESIZE:
            calculateDisplayRects();
        case CCS_REDRAW:
            render();
            break;
        default:
            break;
        }
    }

    if (cmd != CCS_FORCE_EXIT)
    {
        displayExit();

        if (m_console->pause() == CCS_RESTART)
        {
            goto top;
        }
    }
    return m_return;
}

void Debugger::constructDebugInfo()
{
    m_debugInfo.reserve(m_ins.size());

    ExecInstructions::iterator it = m_ins.begin(), end = m_ins.end();
    while (it != end)
    {
        const ExecInstruction& exec = (*it++);

        DebugInstruction dbg;
        dbg.flags = 0;
        dbg.inst  = exec;
        dbg.value = getStrValue(exec);
        m_debugInfo.push_back(dbg);
    }

    // free the unused instructions
    m_ins.clear();
}

void Debugger::calculateDisplayRects(void)
{
    const ConsoleRect& rect = m_console->getRect();

    m_instRect.x = 0;
    m_instRect.y = 0;
    m_instRect.w = getMaxInstructionSize();
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

void Debugger::displayHeader(void)
{
    str_t         string;
    ostringstream stream;

    m_console->setColor(CS_GREEN);

    stream << "Instructions [" << m_debugInfo.size() << "]";
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

void Debugger::displayInstructions(void)
{
    int16_t ypos = m_instRect.y + 1;

    int16_t offs = (int16_t)m_curinst, i;
    int16_t maxy = m_instRect.bottom();

    offs -= maxy - (maxy >> 2);
    if (offs < 0)
        offs = 0;

    for (i = 0; i < maxy; ++i, ++ypos)
    {
        size_t cinst = (size_t)offs + (size_t)i;
        if (cinst < m_debugInfo.size())
            disassemble(m_debugInfo.at(cinst), cinst, ypos);
        else
            disassemble(nop, cinst, ypos);
    }
}

void Debugger::stepOneInstruction(void)
{
    if (m_curinst < m_debugInfo.size())
    {
        bool switchOutput = false;

        const DebugInstruction& dbg = m_debugInfo.at((size_t)m_curinst++);
        if (OPCodeTable[dbg.inst.op] != nullptr)
        {
            switchOutput = (dbg.inst.op == OP_GTO && dbg.inst.call) || (dbg.inst.op >= OP_PRG);
            if (switchOutput)
                m_console->switchOutput(true);

            (this->*OPCodeTable[dbg.inst.op])(dbg.inst);

            if (switchOutput)
                m_console->switchOutput(false);
        }
    }
    else
        m_exit = true;
}

void Debugger::stepToNextBreakPoint(void)
{
    stepOneInstruction();

    bool breakPointHit = false;
    while (!m_exit && !breakPointHit)
    {
        DebugInstruction* dbg = getCurrent();
        if (dbg)
        {
            if (dbg->flags & DF_BREAK)
                breakPointHit = true;
            else
                stepOneInstruction();
        }
        else
            m_exit = true;
    }
}

void Debugger::stepOut(void)
{
    size_t callStackSize = m_callStack.size();
    stepOneInstruction();

    bool breakPointHit = false;
    while (!m_exit && !breakPointHit)
    {
        DebugInstruction* dbg = getCurrent();
        if (dbg)
        {
            if (dbg->flags & DF_BREAK)
                breakPointHit = true;
            else
                stepOneInstruction();
        }
        else
            m_exit = true;

        if (m_callStack.size() < callStackSize)
            breakPointHit = true;
    }
}

void Debugger::addBreakPoint(void)
{
    DebugInstruction* dbg = getCurrent();
    if (dbg)
    {
        if (dbg->flags & DF_BREAK)
            dbg->flags &= ~DF_BREAK;
        else
            dbg->flags |= DF_BREAK;
    }
}

void Debugger::displayRegisters(void)
{
    ostringstream ss, ss1;

    int16_t y;
    int16_t ypos = m_regiRect.y + 1;

    m_console->setColor(CS_WHITE);

    ss << setfill('0') << uppercase << hex;
    for (y = 0; y < MAX_REG; ++y, ++ypos)
    {
        ss << "0x" << m_regi[y].x;
        ss1 << 'x' << y << ' ';
        ss1 << right << setw(19) << ss.str();
        ss1 << ' ' << setw(19);
        ss1 << m_regi[y].x;

        if (m_regi[y].x != m_last[y].x)
            m_console->setColor(CS_RED);
        else
            m_console->setColor(CS_LIGHT_GREY);

        m_last[y].x = m_regi[y].x;
        m_console->displayString(ss1.str(), m_regiRect.x, ypos);

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
    m_console->displayString(ss.str(), m_regiRect.x, ypos);
}

void Debugger::displayStack(void)
{
    int16_t ypos = m_stackRect.y + 1, y;
    if (m_stack.empty())
        return;

    m_console->setColor(CS_LIGHT_GREY);
    int16_t stk = (int16_t)m_stack.size();
    if (stk > 4)
        stk = 4;

    for (y = 0; y < stk; ++y, ++ypos)
    {
        m_console->setColor(CS_GREY);
        m_console->displayChar('I', m_stackRect.x, ypos);
        m_console->displayInt(y, m_stackRect.x + 2, ypos);
        m_console->setColor(CS_WHITE);
        m_console->displayIntRightAligned((int)m_stack.peek(y),
                                          m_stackRect.right() - 1,
                                          ypos);
    }
}

void Debugger::displayData(void)
{
    uint8_t* data = m_dataTable.ptr();
    if (!data || (m_dataTable.capacity() <= 0) || (m_curinst >= m_debugInfo.size()))
        return;

    if (m_baseAddr == 0)
        m_baseAddr = (size_t)data;

    int16_t ypos = m_dataRect.y + 1;

    const DebugInstruction& dbg  = m_debugInfo.at((size_t)m_curinst);
    const ExecInstruction&  inst = dbg.inst;

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
    m_console->setColor(CS_WHITE);
    m_console->displayOutput(m_outRect.x, m_outRect.y + 1);
}

void Debugger::displayExit(void)
{
    if (m_curinst > 0)
        --m_curinst;

    ostringstream ss;
    ss << "\nExited with code " << m_regi[0].w[0] << '\n';

    m_console->appendOutput(ss.str());

    ss.str("");
    ss << "Press (q) to exit or (r) to restart.\n\n";
    m_console->appendOutput(ss.str());
    m_console->getOutputLineCount();
    render();

    m_console->flush();

    // reset all state variables
    clearState();
}

void Debugger::clearState(void)
{
    m_console->clearOutput();

    m_callStack.clear();
    m_stack.clear();

    memset(m_regi, 0, sizeof(Registers));
    memset(m_last, 0, sizeof(Registers));

    m_baseAddr = 0;
    m_lastAddr = -1;
    m_dataTableCpy.cloneInto(m_dataTable);

    m_flags   = 0;
    m_curinst = m_startinst;
    m_exit    = false;
}

void Debugger::disassemble(const DebugInstruction& inst, size_t i, int16_t y)
{
    ostringstream addr, cursor;
    addr << right << setw(4) << setfill('0') << uppercase << hex << i;
    addr << ' ' << setfill(' ');

    if (i == m_curinst)
        m_console->setColor(CS_YELLOW, CS_DARKMAGENTA);
    else if (inst.flags & DF_BREAK)
        m_console->setColor(CS_WHITE, CS_RED);
    else
    {
        if (inst.inst.op == 0)
            m_console->setColor(CS_GREY);
        else
            m_console->setColor(CS_LIGHT_GREY);
    }

    m_console->displayString(addr.str(), m_instRect.x, y);

    if (i == m_curinst)
    {
        cursor << " ==>";
        m_console->setColor(CS_YELLOW, CS_DARKMAGENTA);
    }
    else
    {
        cursor << setw(4) << ' ';
        m_console->setColor(CS_WHITE);
    }
    if (inst.inst.op == 0)
        m_console->setColor(CS_GREY);

    cursor << inst.value;
    m_console->displayString(cursor.str(), m_instRect.x + 4, y);
}

str_t Debugger::getStrValue(const ExecInstruction& inst)
{
    InstructionWriter cw(inst);
    cw.writeOp();

    switch (inst.op)
    {
    case OP_RET:
    case OP_PRI:
        break;
    case OP_MOV:
        if (inst.flags & IF_INSP)
            cw.writePC();
        else
            cw.writeRegister(0);
        cw.writeNext();
        if (inst.flags & IF_REG1)
            cw.writeRegister(1);
        else
            cw.writeValue(1);
        break;
    case OP_GTO:
    case OP_JMP:
    case OP_JEQ:
    case OP_JNE:
    case OP_JLT:
    case OP_JGT:
    case OP_JLE:
    case OP_JGE:
        if (inst.flags & IF_SYMU)
            cw.writeCall();
        else
            cw.writeValue(0);
        break;
    case OP_PRG:
    case OP_INC:
    case OP_DEC:
        cw.writeRegister(0);
        break;
    case OP_CMP:
        if (inst.flags & IF_REG0)
            cw.writeRegister(0);
        else
            cw.writeValue(0);
        cw.writeNext();
        if (inst.flags & IF_REG1)
            cw.writeRegister(1);
        else
            cw.writeValue(1);
        break;

    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_SHR:
    case OP_SHL:
        if (inst.flags & IF_REG0)
            cw.writeRegister(0);
        else
            cw.writeValue(0);
        cw.writeNext();
        if (inst.flags & IF_REG1)
            cw.writeRegister(1);
        else
            cw.writeValue(1);

        if (inst.argc > 2)
        {
            if (inst.op == OP_ADD)
            {
                cw.writeNext();
                if (inst.flags & IF_ADRD)
                    cw.writeAddrD(m_dataTable.addr(inst.argv[2]));
                else
                    cw.writeValue(2);
            }
            else
            {
                cw.writeNext();
                if (inst.flags & IF_REG2)
                    cw.writeRegister(2);
                else
                    cw.writeValue(2);
            }
        }
        break;
    case OP_ADRP:
        cw.writeRegister(0);
        cw.writeNext();
        cw.writeAddrD(m_dataTable.addr(inst.argv[1]));
        break;
    case OP_STP:
    case OP_LDP:
        cw.writeSP();
        cw.writeNext();
        cw.writeValue(1);
        break;
    case OP_LDR:
    case OP_STR:
        cw.writeRegister(0);
        cw.writeNext();

        cw.openBrace();
        if (inst.flags & IF_STKP)
            cw.writeSP();
        else
            cw.writeRegister(1);

        cw.writeNext();
        cw.writeIndex();
        cw.closeBrace();
        break;

    case OP_LDRS:
    case OP_STRS:

        cw.writeRegister(0);
        cw.writeNext();

        cw.openBrace();
        if (inst.flags & IF_STKP)
            cw.writeSP();
        else
            cw.writeRegister(1);

        cw.writeNext();
        cw.writeRegIndex();
        cw.closeBrace();
        break;
    }
    return cw.string();
}

int16_t Debugger::getMaxInstructionSize(void)
{
    if (m_maxInstWidth <= 0)
    {
        DebugInstructions::iterator it = m_debugInfo.begin(), end = m_debugInfo.end();
        while (it != end)
        {
            const DebugInstruction& dbg = (*it++);

            if (m_maxInstWidth < dbg.value.size())
                m_maxInstWidth = (int16_t)dbg.value.size();
        }

        // 4 for the address
        // 4 for the cursor
        // 2 padding
        m_maxInstWidth += 10;
    }
    return m_maxInstWidth;
}

DebugInstruction* Debugger::getCurrent(void)
{
    if (m_curinst < m_debugInfo.size())
        return &m_debugInfo[m_curinst];
    return nullptr;
}

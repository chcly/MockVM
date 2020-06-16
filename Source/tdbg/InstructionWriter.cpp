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
#include "InstructionWriter.h"
#include <iomanip>
using namespace std;

InstructionWriter::InstructionWriter(const ExecInstruction &inst) :
    m_inst(inst)
{
    m_os << uppercase << hex;
}

void InstructionWriter::writeOp(void)
{
    m_os << left << setw(6);
    switch (m_inst.op)
    {
    case OP_RET:
        m_os << "ret";
        break;
    case OP_MOV:
        m_os << "mov";
        break;
    case OP_GTO:
        m_os << "bl";
        break;
    case OP_INC:
        m_os << "inc";
        break;
    case OP_DEC:
        m_os << "dec";
        break;
    case OP_CMP:
        m_os << "cmp";
        break;
    case OP_JMP:
        m_os << "b";
        break;
    case OP_JEQ:
        m_os << "beq";
        break;
    case OP_JNE:
        m_os << "bne";
        break;
    case OP_JLT:
        m_os << "blt";
        break;
    case OP_JGT:
        m_os << "bgt";
        break;
    case OP_JLE:
        m_os << "ble";
        break;
    case OP_JGE:
        m_os << "bge";
        break;
    case OP_ADD:
        m_os << "add";
        break;
    case OP_SUB:
        m_os << "sub";
        break;
    case OP_MUL:
        m_os << "mul";
        break;
    case OP_DIV:
        m_os << "div";
        break;
    case OP_SHR:
        m_os << "shr";
        break;
    case OP_SHL:
        m_os << "shl";
        break;
    case OP_ADRP:
        m_os << "adrp";
        break;
    case OP_STR:
        m_os << "str";
        break;
    case OP_LDR:
        m_os << "ldr";
        break;
    case OP_LDRS:
        m_os << "ldrs";
        break;
    case OP_STRS:
        m_os << "strs";
        break;
    case OP_STP:
        m_os << "stp";
        break;
    case OP_LDP:
        m_os << "ldp";
        break;
    case OP_PRG:
        m_os << "prg";
        break;
    case OP_PRI:
        m_os << "prgi";
        break;
    default:
        m_os << "nop";
        break;
    }
}

void InstructionWriter::writeSpace(void)
{
    m_os << ' ';
}

void InstructionWriter::writePC(void)
{
    m_os << "pc";
}

void InstructionWriter::writeSP(void)
{
    m_os << "sp";
}

void InstructionWriter::writeRegister(int index)
{
    if (index < m_inst.argc)
        m_os << 'x' << m_inst.argv[index];
}

void InstructionWriter::writeValue(int index, int width)
{
    if (index < m_inst.argc)
    {
        if (width != -1)
        {
            m_os << setfill('0') << right;
            m_os << "0x" << setw(4) << m_inst.argv[index];
            m_os << setfill(' ') << left;
        }
        else
            m_os << "0x" << m_inst.argv[index];
    }
}

void InstructionWriter::writeIndex(void)
{
    m_os << dec;
    m_os << m_inst.index;
    m_os << hex;
}

void InstructionWriter::writeRegIndex(void)
{
    m_os << dec;
    m_os << 'x' << m_inst.index;
    m_os << hex;
}

void InstructionWriter::writeCall(void)
{
    m_os << "0x" << (size_t)m_inst.call;
}

void InstructionWriter::writeAddrD(size_t v)
{
    m_os << "0x" << v;
}

void InstructionWriter::writeNext(void)
{
    m_os << ", ";
}

void InstructionWriter::openBrace(void)
{
    m_os << '[';
}

void InstructionWriter::closeBrace(void)
{
    m_os << ']';
}

str_t InstructionWriter::string(void)
{
    return m_os.str();
}

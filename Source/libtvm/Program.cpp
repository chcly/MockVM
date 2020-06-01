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
#include <string.h>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include "BlockReader.h"
#include "Declarations.h"
#include "SharedLib.h"
#include "SymbolUtils.h"

using namespace std;

const size_t MaxRegisterSize = sizeof(Register) * (MAX_REG - 1);

Program::Program(const str_t& modpath) :
    m_flags(0),
    m_return(0),
    m_modpath(modpath)
{
    memset(m_regi, 0, sizeof(Registers));
    m_stack.reserve(256);
    m_callStack.reserve(256);
}

Program::~Program()
{
    DynamicLib::iterator it = m_dynlib.begin();
    while (it != m_dynlib.end())
        UnloadSharedLibrary(*it++);
}

int Program::load(const char* fname)
{
    if (!fname)
    {
        printf("invalid file path name\n");
        return PS_ERROR;
    }

    BlockReader reader = BlockReader(fname);
    if (reader.eof())
    {
        printf("failed to load '%s'\n", fname);
        return PS_ERROR;
    }

    reader.read(&m_header, sizeof(TVMHeader));
    if (m_header.code[0] != 'T' || m_header.code[1] != 'V')
    {
        printf("invalid file type identifier\n");
        return PS_ERROR;
    }

    if (m_header.str != 0)
    {
        if (loadStringTable(reader) != PS_OK)
        {
            printf("failed to read the string table\n");
            return PS_ERROR;
        }
    }

    if (m_header.dat != 0)
    {
        if (loadDataTable(reader) != PS_OK)
        {
            printf("failed to read the data table\n");
            return PS_ERROR;
        }
    }

    if (m_header.sym != 0)
    {
        if (loadSymbolTable(reader) != PS_OK)
        {
            printf("failed to read the symbol table\n");
            return PS_ERROR;
        }
    }

    if (loadCode(reader) != PS_OK)
    {
        printf("failed to read the file's instruction table\n");
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

    str_t    str;
    uint32_t i, st = PS_OK;
    size_t   tot = 0;
    for (i = 0; i < strTab.size && !reader.eof(); ++i)
    {
        char ch = reader.next();
        if (ch >= 32 && ch <= 127)
            str.push_back(ch);
        else if (ch != 0)
        {
            printf("unknown character '%c' was found in the string table\n", ch);
            st = PS_ERROR;
            i  = strTab.size;
        }
        else
        {
            if (!str.empty())
            {
                if (m_strtab.find(str) != m_strtab.end())
                {
                    printf("duplicate string '%s' was found in the string table\n",
                           str.c_str());
                    st = PS_ERROR;
                    i  = strTab.size;
                }
                else
                {
                    m_strtab[str] = tot++;
                    m_strtablist.push_back(str);
                    str.resize(0);
                }
            }
            else
            {
                // force an early exit even if
                // the whole string table has
                // not been read.
                if (i + 1 != strTab.size)
                    st = PS_ERROR;
                i = strTab.size;
            }
        }
    }
    return st;
}

int Program::loadSymbolTable(BlockReader& reader)
{
    reader.moveTo(m_header.sym);
    TVMSection symtab;
    reader.read(&symtab, sizeof(TVMSection));

    if (symtab.size <= 0)
        return PS_OK;

    str_t    str;
    uint32_t i, st = PS_OK;
    size_t   tot = 0;

    for (i = 0; i < symtab.size && !reader.eof(); ++i)
    {
        char ch = reader.next();
        if (ch >= 32 && ch <= 127)
            str.push_back(ch);
        else if (ch != 0)
        {
            printf("unknown character '%c' was found in the string table\n", ch);
            st = PS_ERROR;
            i  = symtab.size;
        }
        else
        {
            if (!str.empty())
            {
                LibHandle lib = nullptr;
                if (IsModulePresent(str, m_modpath))
                {
                    lib = LoadSharedLibrary(str, m_modpath);
                    if (lib != nullptr)
                        m_dynlib.push_back(lib);
                }

                if (!lib)
                {
                    printf("failed to locate the file '%s' in the module directory '%s'\n",
                           str.c_str(),
                           m_modpath.c_str());
                    st = PS_ERROR;
                    i  = symtab.size;
                }
                str.resize(0);
            }
            else
            {
                // force an early exit even if
                // the whole string table has
                // not been read.
                if (i + 1 != symtab.size)
                    st = PS_ERROR;
                i = symtab.size;
            }
        }
    }
    return st;
}

int Program::loadDataTable(BlockReader& reader)
{
    reader.moveTo(m_header.dat);
    TVMSection dat;
    reader.read(&dat, sizeof(TVMSection));

    if (dat.size <= 0)
        return PS_OK;

    m_dataTable.reserve((size_t)dat.size + (size_t)dat.align);
    reader.read(m_dataTable.ptr(), m_dataTable.capacity());
    return PS_OK;
}

int Program::loadCode(BlockReader& reader)
{
    reader.moveTo(sizeof(TVMHeader));
    TVMSection code;
    reader.read(&code, sizeof(TVMSection));

    if (code.size <= 0)
        return PS_OK;

    uint16_t sizes = 0;
    uint8_t  v8;
    uint16_t v16;
    uint32_t v32;
    int      a;
    size_t   i = 0;
    while (i < code.size && !reader.eof())
    {
        ExecInstruction exec = {};

        i += reader.read(&exec.op, 2);
        i += reader.read(&exec.flags, 2);
        i += reader.read(&sizes, 2);

        if (exec.flags & IF_RIDX)
            i += reader.read(&exec.index, 1);

        for (a = 0; a < exec.argc && a < INS_ARG; ++a)
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
        }

        if (exec.flags & IF_SYMU)
        {
            if (findDynamic(exec) != PS_OK)
            {
                printf("failed to locate symbol\n");
                return PS_ERROR;
            }
        }

        if (testInstruction(exec))
            m_ins.push_back(exec);
        else
            return PS_ERROR;
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

int Program::findDynamic(ExecInstruction& ins)
{
    if (ins.argv[0] < m_strtablist.size())
    {
        str_t name = m_strtablist.at(ins.argv[0]), look;

        // This needs to change to something better.
        // It should be a predictable identifier to look up
        // exported functions by the symbol itself rather
        // than having to iterate over a table to find a named
        // symbol
        look = "__" + name;

        SymbolMap::iterator it = m_symbols.find(name);
        if (it == m_symbols.end())
        {
            Symbol search = nullptr;

            DynamicLib::iterator it = m_dynlib.begin();
            while (it != m_dynlib.end() && search == nullptr)
            {
                LibHandle lib = (*it++);
                LibSymbol sym = GetSymbolAddress(lib, look.c_str());

                if (sym != nullptr)
                {
                    search          = (Symbol)sym;
                    m_symbols[name] = search;
                }
            }

            ins.call = search;
        }
        else
            ins.call = it->second;
    }

    return ins.call != nullptr ? (int)PS_OK : (int)PS_ERROR;
}

int Program::launch(void)
{
    if (m_ins.empty())
        return PS_OK;

    size_t tinst = m_ins.size();

    const ExecInstruction* basePtr = m_ins.data();
    m_callStack.push(m_curinst);

    while (m_curinst < tinst)
    {
        const ExecInstruction& inst = basePtr[m_curinst++];
        if (inst.op > OP_BEG && inst.op < OP_MAX)
        {
            if (OPCodeTable[inst.op] != nullptr)
                (this->*OPCodeTable[inst.op])(inst);
        }
    }
    if (m_return == -1)
        printf("an error occurred\n");
    return m_return;
}

void Program::forceExit(int returnCode)
{
    m_return  = returnCode;
    m_curinst = m_ins.size();
}

Register* Program::clone(void)
{
    Register* reg = new Register[MAX_REG];
    memcpy(reg, m_regi, MaxRegisterSize);
    return reg;
}

void Program::release(Register* reg)
{
    memcpy(m_regi, reg, MaxRegisterSize);
    delete[] reg;
}

void Program::handle_OP_RET(const ExecInstruction& inst)
{
    if (!m_callStack.empty())
    {
        m_curinst = m_callStack.top();
        m_callStack.pop();
    }

    if (m_callStack.empty())
        m_curinst = m_ins.size();
    m_return = (int32_t)m_regi[0].w[0];
}

void Program::handle_OP_MOV(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        if (inst.flags & IF_INSP)
        {
            if (inst.flags & IF_REG1)
                m_curinst = m_regi[inst.argv[1]].x;
            else
                m_curinst = inst.argv[1];
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[inst.argv[0]].x = m_regi[inst.argv[1]].x;
            else
            {
                if (inst.flags & IF_BTEB)
                    m_regi[inst.argv[0]].b[0] = (uint8_t)inst.argv[1];
                else if (inst.flags & IF_BTEW)
                    m_regi[inst.argv[0]].w[0] = (uint16_t)inst.argv[1];
                else if (inst.flags & IF_BTEL)
                    m_regi[inst.argv[0]].l[0] = (uint32_t)inst.argv[1];
                else
                    m_regi[inst.argv[0]].x = inst.argv[1];
            }
        }
    }
}

void Program::handle_OP_CALL(const ExecInstruction& inst)
{
    if (inst.flags & IF_SYMU)
    {
        if (inst.call != nullptr)
        {
            // This does not guard against corrupting this
            // pointer, but it allows access to the registers
            // without passing the address of m_regi
            // which can then be used to access internal
            // class members.
            Register* cl = clone();
            inst.call((tvmregister_t)cl);
            release(cl);
        }
    }
    else if (inst.flags & IF_ADDR)
    {
        m_callStack.push(m_curinst);
        m_curinst = inst.argv[0];
        if (m_callStack.size() > MAX_STK)
        {
            printf("maximum number of branches exceeded.\n");
            forceExit(-1);
        }
    }
    else
    {
        printf("unknown call flag\n");
        forceExit(-1);
    }
}

void Program::handle_OP_INC(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
        m_regi[inst.argv[0]].x += 1;
}

void Program::handle_OP_DEC(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
        m_regi[inst.argv[0]].x -= 1;
}

void Program::handle_OP_CMP(const ExecInstruction& inst)
{
    uint64_t a = inst.argv[0];
    uint64_t b = inst.argv[1];

    if (inst.flags & IF_REG0)
        a = m_regi[a].x;
    if (inst.flags & IF_REG1)
        b = m_regi[b].x;

    m_flags   = 0;
    int64_t r = (int64_t)a - (int64_t)b;
    if (r == 0)
        m_flags |= PF_Z;
    else if (r < 0)
        m_flags |= PF_L;
    else if (r > 0)
        m_flags |= PF_G;
}

void Program::handle_OP_JMP(const ExecInstruction& inst)
{
    m_curinst = inst.argv[0];
}

void Program::handle_OP_JEQ(const ExecInstruction& inst)
{
    if (m_flags & PF_Z)
    {
        m_flags &= ~PF_Z;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JNE(const ExecInstruction& inst)
{
    if ((m_flags & PF_Z) == 0)
    {
        m_flags &= ~PF_Z;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JLE(const ExecInstruction& inst)
{
    if (m_flags & PF_Z)
    {
        m_flags &= ~PF_Z;
        m_curinst = inst.argv[0];
    }
    else if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JGE(const ExecInstruction& inst)
{
    if (m_flags & PF_Z)
    {
        m_flags &= ~PF_Z;
        m_curinst = inst.argv[0];
    }
    else if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JLT(const ExecInstruction& inst)
{
    if (m_flags & PF_L)
    {
        m_flags &= ~PF_L;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_JGT(const ExecInstruction& inst)
{
    if (m_flags & PF_G)
    {
        m_flags &= ~PF_G;
        m_curinst = inst.argv[0];
    }
}

void Program::handle_OP_ADD(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];
        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            if (inst.flags & IF_ADRD)
            {
                // meaning the address in b, should be dereferenced
                uint8_t* ptr = (uint8_t*)(size_t)m_regi[b].x;
                if (ptr)
                {
                    uint64_t* c  = (uint64_t*)ptr;
                    m_regi[x0].x = *c;
                }
            }
            else
            {
                uint64_t c = inst.argv[2];
                if (inst.flags & IF_REG1)
                    b = m_regi[b].x;
                if (inst.flags & IF_REG2)
                    c = m_regi[c].x;

                m_regi[x0].x = b + c;
            }
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[x0].x += m_regi[inst.argv[1]].x;
            else
                m_regi[x0].x += inst.argv[1];
        }
    }
}

void Program::handle_OP_SUB(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];
        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];

            if (inst.flags & IF_REG1)
                b = m_regi[b].x;
            if (inst.flags & IF_REG2)
                c = m_regi[c].x;
            m_regi[x0].x = b - c;
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[x0].x -= m_regi[inst.argv[1]].x;
            else
                m_regi[x0].x -= inst.argv[1];
        }
    }
}

void Program::handle_OP_MUL(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];
        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];

            if (inst.flags & IF_REG1)
                b = m_regi[b].x;
            if (inst.flags & IF_REG2)
                c = m_regi[c].x;
            m_regi[x0].x = b * c;
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[x0].x *= m_regi[inst.argv[1]].x;
            else
                m_regi[x0].x *= inst.argv[1];
        }
    }
}

void Program::handle_OP_DIV(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];

        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];

            if (inst.flags & IF_REG1)
                b = m_regi[b].x;
            if (inst.flags & IF_REG2)
                c = m_regi[c].x;

            if (c != 0)
                m_regi[x0].x = b / c;
            else
            {
                printf("divide by zero\n");
                forceExit(-1);
            }
        }
        else
        {
            if (inst.flags & IF_REG1)
            {
                if (m_regi[inst.argv[1]].x != 0)
                    m_regi[x0].x /= m_regi[inst.argv[1]].x;
                else
                {
                    printf("divide by zero\n");
                    forceExit(-1);
                }
            }
            else
            {
                if (inst.argv[1] != 0)
                    m_regi[x0].x /= inst.argv[1];
                else
                {
                    printf("divide by zero\n");
                    forceExit(-1);
                }
            }
        }
    }
}

void Program::handle_OP_SHR(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];

        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];

            if (inst.flags & IF_REG1)
                b = m_regi[b].x;
            if (inst.flags & IF_REG2)
                c = m_regi[c].x;

            m_regi[x0].x = b >> c;
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[x0].x >>= m_regi[inst.argv[1]].x;
            else
                m_regi[x0].x >>= inst.argv[1];
        }
    }
}

void Program::handle_OP_SHL(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        const uint64_t& x0 = inst.argv[0];

        if (inst.argc > 2)
        {
            uint64_t b = inst.argv[1];
            uint64_t c = inst.argv[2];

            if (inst.flags & IF_REG1)
                b = m_regi[b].x;
            if (inst.flags & IF_REG2)
                c = m_regi[c].x;

            m_regi[x0].x = b << c;
        }
        else
        {
            if (inst.flags & IF_REG1)
                m_regi[x0].x <<= m_regi[inst.argv[1]].x;
            else
                m_regi[x0].x <<= inst.argv[1];
        }
    }
}

void Program::handle_OP_ADRP(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        if (inst.argv[1] < m_dataTable.capacity())
        {
            uint8_t* base          = m_dataTable.ptr();
            m_regi[inst.argv[0]].x = (size_t)(&base[inst.argv[1]]);
        }
    }
}

void Program::handle_OP_STP(const ExecInstruction& inst)
{
    if (inst.flags & IF_STKP)
    {
        uint64_t nrel = inst.argv[1] / 8;
        if (nrel > 32)
        {
            // stp sp, > 256
            printf("Stack size exceeded\n");
            forceExit(-1);
        }
        else
        {
            if (m_stack.size() >= MAX_STK)
            {
                printf("stack overflow.\n");
                forceExit(-2);
            }
            else
            {
                uint64_t i;
                for (i = 0; i < nrel; ++i)
                    m_stack.push(0);
            }
        }
    }
}

void Program::handle_OP_LDP(const ExecInstruction& inst)
{
    if (inst.flags & IF_STKP)
    {
        uint64_t nrel = inst.argv[1] / 8; 
        if (nrel > 32)
        {
            // stp sp, > 256
            printf("stack size exceeded\n");
            forceExit(-1);
        }
        else
        {
            uint64_t i;
            for (i = 0; i < nrel && !m_stack.empty(); ++i)
                m_stack.pop();
        }
    }
}

void Program::handle_OP_STR(const ExecInstruction& inst)
{
    if (inst.flags & IF_STKP)
    {
        uint64_t nrel = inst.argv[1] / 8;
        if (nrel > 32)
        {
            printf("Stack size exceeded\n");
            forceExit(-1);
        }
        else
        {
            // o1 -> o2

            size_t stk = m_stack.size();
            size_t idx = (inst.index / 8);
            size_t rem = (inst.index % 8);

            if (inst.flags & IF_REG0)
            {
                if (idx < stk)
                {
                    uint64_t& dest = m_stack.peek(idx);
                    if (rem == 0)
                        dest = m_regi[inst.argv[0]].x;
                    else
                    {
                        // place elsewhere in the register
                    }
                }
            }
        }
    }
}

void Program::handle_OP_LDR(const ExecInstruction& inst)
{
    if (inst.flags & IF_STKP)
    {
        uint64_t nrel = inst.argv[1] / 8;
        if (nrel > 32)
        {
            printf("Stack size exceeded\n");
            forceExit(-1);
        }
        else
        {
            // o1 <- o2
            size_t stk = m_stack.size();
            size_t idx = (inst.index / 8);
            size_t rem = (inst.index % 8);

            if (inst.flags & IF_REG0)
            {
                if (idx < stk)
                {
                    const uint64_t& src = m_stack.peek(idx);
                    if (rem == 0)
                        m_regi[inst.argv[0]].x = src;
                    else
                    {
                        // place elsewhere in the register
                    }
                }
            }
        }
    }
    else if (inst.flags & IF_REG1)
    {
        // o1 <- o2
        Register& dest = m_regi[inst.argv[0]];
        if (inst.flags & IF_BTEB)
        {
            if (inst.index < 8)
                dest.b[inst.index] = m_regi[inst.argv[1]].b[inst.index];
        }
        else if (inst.flags & IF_BTEW)
        {
            if (inst.index < 4)
                dest.w[inst.index] = m_regi[inst.argv[1]].w[inst.index];
        }
        else if (inst.flags & IF_BTEL)
        {
            if (inst.index < 2)
                dest.l[inst.index] = m_regi[inst.argv[1]].l[inst.index];
        }
        else
            dest.x = m_regi[inst.argv[1]].x;
    }
}

void Program::handle_OP_PRG(const ExecInstruction& inst)
{
    if (inst.flags & IF_REG0)
    {
        cout << (int64_t)m_regi[inst.argv[0]].x << std::endl;
    }
    else
    {
        cout << (int64_t)inst.argv[0] << std::endl;
    }
}

void Program::handle_OP_PRGI(const ExecInstruction& inst)
{
    int i;

    stringstream ss;
    for (i = 0; i < MAX_REG; ++i)
    {
        ss << "0x" << setfill('0') << uppercase << hex << m_regi[i].x;
        cout << ' ' << 'x' << left << setw(4) << i << ' ';
        cout << right << setw(17) << ss.str();
        cout << ' ';
        cout << dec;
        cout << setw(22);
        cout << m_regi[i].x;
        cout << endl;
        ss.str("");
    }
}

bool Program::testInstruction(const ExecInstruction& exec)
{
    bool pass = exec.op > OP_BEG && exec.op < OP_MAX;
    if (!pass)
    {
        printf("instruction boundary exceeded\n");
        return false;
    }

    switch (exec.op)
    {
    case OP_PRI:
    case OP_RET:
        pass = exec.argc == 0;
        break;
    case OP_GTO:
    case OP_INC:
    case OP_DEC:
    case OP_PRG:
    case OP_JMP:
    case OP_JGE:
    case OP_JLE:
    case OP_JGT:
    case OP_JLT:
    case OP_JEQ:
    case OP_JNE:
        pass = exec.argc == 1;
        break;
    case OP_MOV:
    case OP_CMP:
    case OP_ADRP:
    case OP_STR:
    case OP_LDR:
    case OP_STP:
    case OP_LDP:
        pass = exec.argc == 2;
        break;
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_SHR:
    case OP_SHL:
        pass = exec.argc == 2 || exec.argc == 3;
        break;
    default:
        pass = false;
        break;
    }

    if (!pass)
    {
        printf("invalid argument count\n");
        return false;
    }

    switch (exec.op)
    {
    case OP_RET:
    case OP_PRI:
        break;
    case OP_JGE:
    case OP_JLE:
    case OP_JGT:
    case OP_JLT:
    case OP_JEQ:
    case OP_JNE:
    case OP_JMP:
        pass = (exec.flags & IF_ADDR) != 0;
        break;
    case OP_GTO:
        pass = (exec.flags & IF_ADDR) != 0;
        if (!pass)
            pass = (exec.flags & IF_SYMU) != 0;
        break;
    case OP_PRG:
        if (exec.flags & IF_REG0)
            pass = exec.argv[0] < MAX_REG;
        break;
    case OP_DEC:
    case OP_INC:
        pass = (exec.flags & IF_REG0) != 0;
        if (pass)
            pass = exec.argv[0] < MAX_REG;
        break;
    case OP_CMP:
        if (exec.flags & IF_REG0)
            pass = exec.argv[0] < MAX_REG;
        if (exec.flags & IF_REG1)
            pass = exec.argv[1] < MAX_REG;
        break;
    case OP_ADRP:
        pass = (exec.flags & IF_REG0) != 0;
        if (pass)
        {
            pass = exec.argv[0] < MAX_REG;
            if (pass)
                pass = (exec.flags & IF_ADRD) != 0;
        }
        break;
    case OP_MOV:
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV:
    case OP_SHR:
    case OP_SHL:
    case OP_STR:
    case OP_LDR:
    case OP_STP:
    case OP_LDP:
        pass = (exec.flags & IF_REG0) != 0;
        if (pass)
        {
            pass = exec.argv[0] < MAX_REG;
            if (pass)
            {
                if (exec.flags & IF_REG1 || exec.flags & IF_RIDX)
                    pass = exec.argv[1] < MAX_REG;
            }

            if (pass)
            {
                if (exec.flags & IF_ADRD)
                    pass = exec.argv[2] < m_dataTable.capacity();
                else if (exec.flags & IF_REG2)
                    pass = exec.argv[2] < MAX_REG;
            }
        }
        break;
    default:
        pass = false;
        break;
    }
    if (!pass)
    {
        printf("invalid instruction\n");
        return false;
    }
    return true;
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
    &Program::handle_OP_ADRP,
    &Program::handle_OP_STR,
    &Program::handle_OP_LDR,
    &Program::handle_OP_STP,
    &Program::handle_OP_LDP,
    &Program::handle_OP_PRG,
    &Program::handle_OP_PRGI,
};

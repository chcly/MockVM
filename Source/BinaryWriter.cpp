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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "BinaryWriter.h"
#include "SymbolUtils.h"
#include <stdio.h>
#include <iostream>
#include <limits>

SYM_EXPORT SymbolMapping* std_init();


using namespace std;

inline uint32_t getAlignment(size_t al)
{
    uint32_t rem = (al % 16);
    if (rem > 0)
        return (16 - rem);
    return 0;
}


BinaryWriter::BinaryWriter() :
    m_fp(0),
    m_loc(0),
    m_ins(),
    m_sizeOfCode(0),
    m_sizeOfData(0),
    m_sizeOfSym(0),
    m_sizeOfStr(0),
    m_stdlib(),
    m_addrMap(),
    m_labels(),
    m_header({})
{
    m_stdlib = std_init();
}


BinaryWriter::~BinaryWriter()
{
    if (m_fp)
        fclose((FILE*)m_fp);
}

void BinaryWriter::mergeInstructions(const Instructions& insl)
{
    m_ins.reserve(insl.size());
    for (Instruction ins : insl)
        m_ins.push_back(ins);
}

void BinaryWriter::mergeLabels(const LabelMap& map)
{
    LabelMap::const_iterator it = map.begin();
    while (it != map.end())
    {
        if (m_labels.find(it->first) != m_labels.end())
            cout << "Duplicate label " << it->first << '\n';
        else
            m_labels[it->first] = it->second;
        ++it;
    }
}

void BinaryWriter::write(const void* v, size_t size)
{
    if (m_fp)
        fwrite(v, 1, size, (FILE*)m_fp);
}

void BinaryWriter::write8(uint8_t v)
{
    write(&v, sizeof(uint8_t));
}

void BinaryWriter::write16(uint16_t v)
{
    write(&v, sizeof(uint16_t));
}

void BinaryWriter::write32(uint32_t v)
{
    write(&v, sizeof(uint32_t));
}

void BinaryWriter::write64(uint64_t v)
{
    write(&v, sizeof(uint64_t));
}

int BinaryWriter::open(const char* fname)
{
    if (m_fp)
        fclose((FILE*)m_fp);

    m_fp = fopen(fname, "wb");
    if (!m_fp)
    {
        printf("Failed to open '%s' for writing.\n", fname);
        return PS_ERROR;
    }
    return PS_OK;
}

size_t BinaryWriter::addToStringTable(const str_t& symname)
{
    LabelMap::iterator it = m_strtab.find(symname);
    if (it != m_strtab.end())
        return it->second;


    m_sizeOfStr += symname.size();
    m_sizeOfStr += 1;

    size_t size = m_strtab.size();
    m_strtab[symname] = size;

    m_orderedString.push_back(symname);
    return size;
}

void BinaryWriter::mapInstructions(void)
{
    uint64_t label = PS_UNDEFINED;
    int64_t  insp  = 0;
    int64_t lookup = 0;

    using InstPtr = std::vector<Instruction*>;

    InstPtr symbols;

    m_addrMap.clear();

    Instructions::iterator it = m_ins.begin();
    while (it != m_ins.end())
    {
        Instruction& ins = (*it++);

        // look for changes in the label index then
        // save the index of the first change.
        if (ins.label != label)
        {
            label            = ins.label;
            m_addrMap[label] = insp;
        }

        // store it so it can be resolved 
        // after all labels have been indexed
        if (!ins.lname.empty())
            symbols.push_back(&ins);

        ++insp;
    }

    InstPtr::iterator symit = symbols.begin();
    while (symit != symbols.end())
    {
        Instruction* irp = (*symit++);
        
        // modify the first argument so that it points
        // to the correct instruction index.
        lookup = findLabel(irp->lname); 
        if (lookup != PS_UNDEFINED)
        {
            // It points to a local label
            irp->argv[0] = lookup;
            irp->flags |= IF_ADDR;
        }
        else
        {
            SymbolMapping* map = findStatic(*irp);
            if (map != nullptr)
            {
                // It points to a known symbol
                irp->argv[0] = addToStringTable(irp->lname);
                irp->flags |= IF_SYMA;
            }
            else
            {
                // It points to an unknown symbol
                // that may reside in a shared library
                irp->argv[0] = addToStringTable(irp->lname);
                irp->flags |= IF_SYMU;  
            }
        }
    }
}


size_t BinaryWriter::calculateInstructionSize(void)
{
    size_t i;
    size_t size = 0;

    using lim8  = std::numeric_limits<uint8_t>;
    using lim16 = std::numeric_limits<uint16_t>;
    using lim32 = std::numeric_limits<uint32_t>;

    Instructions::iterator it = m_ins.begin();
    while (it != m_ins.end())
    {
        Instruction& ins = (*it++);
        ins.sizes = 0;

        size += 5;  // op, nr, flags, sizes
        for (i = 0; i < ins.argc; ++i)
        {
            if (ins.argv[i] < lim8().max())
            {
                ins.sizes |= SizeFlags[i][0];
                size += 1;
            }
            else if (ins.argv[i] < lim16().max())
            {
                ins.sizes |= SizeFlags[i][1];
                size += 2;
            }
            else if (ins.argv[i] < lim32().max())
            {
                ins.sizes |= SizeFlags[i][2];
                size += 4;
            }
            else
            {
                size += 8;
            }
        }
    }
    return size;
}


int64_t BinaryWriter::findLabel(const str_t& name)
{
    if (!name.empty())
    {
        LabelMap::iterator it = m_labels.find(name);
        if (it != m_labels.end())
        {
            IndexToPosition::iterator fidx = m_addrMap.find(it->second);
            if (fidx != m_addrMap.end())
                return fidx->second;
        }
    }

    return PS_UNDEFINED;
}

int BinaryWriter::resolve(strvec_t& modules)
{
    int status = PS_OK;

    strvec_t::iterator it = modules.begin();
    while (it != modules.end() && status == PS_OK)
    {
        const str_t& mod = (*it++);

        LibHandle lib = LoadSymbolLibrary(mod.c_str());
        if (lib != nullptr)
        {
            str_t lookup = mod + "_init";
            LibSymbol sym = GetSymbolAddress(lib, lookup.c_str());
            if (sym != nullptr)
            {
                SymbolMapping* avail = ((ModuleInit)sym)();
                if (avail != nullptr)
                {
                    // search symbols from the shared library
                }
            }
            else
            {
                printf("failed to find symbol %s\n", lookup.c_str());
                status = PS_ERROR;
            }

            UnloadSymbolLibrary(lib);
        }
        else
        {
            printf("failed to find library %s\n", mod.c_str());
            status = PS_ERROR;
        }
    }
    return status;
}


SymbolMapping* BinaryWriter::findStatic(const Instruction& ins)
{
    SymbolMapping* val = nullptr;
    if (m_stdlib)
    {
        int i = 0;
        while (m_stdlib[i].name != nullptr && !val)
        {
            SymbolMapping* symbol = &m_stdlib[i++];
            if (ins.lname == symbol->name)
                val = symbol;
        }
    }
    return val;
}



int BinaryWriter::writeHeader()
{
    if (!m_fp)
        return PS_ERROR;

    m_header.code  = TYPE_ID2('T', 'V');
    m_header.flags = 0;

    size_t offset = sizeof(TVMHeader);

    // The text starts at the end of the file header
    m_header.txt = (uint8_t)offset;

    mapInstructions();
    m_sizeOfCode = calculateInstructionSize();
    offset += sizeof(TVMSection);
    offset += m_sizeOfCode;
    offset += getAlignment(m_sizeOfCode);

    m_header.dat = (uint32_t)offset;
    offset += sizeof(TVMSection);
    offset += m_sizeOfData;
    offset += getAlignment(m_sizeOfData);


    // store references to modules
    m_header.sym = offset;
    offset += sizeof(TVMSection);
    offset += m_sizeOfSym;
    offset += getAlignment(m_sizeOfSym);

    m_header.str = offset;

    write(&m_header, sizeof(TVMHeader));
    return PS_OK;
}


size_t BinaryWriter::writeDataSection(void)
{
    TVMSection sec = {};
    sec.size       = (uint32_t)m_sizeOfData;
    sec.code       = TYPE_ID2('D', 'S');
    sec.entry      = m_header.dat;
    sec.align      = 0;
    write(&sec, sizeof(TVMSection));
    return m_sizeOfData;
}

size_t BinaryWriter::writeCodeSection(void)
{
    TVMSection sec = {};
    sec.size       = (uint32_t)m_sizeOfCode;
    sec.code       = TYPE_ID2('C', 'S');
    sec.start      = m_header.txt;
    sec.entry      = findLabel("main");
    sec.align      = (uint32_t)getAlignment(m_sizeOfCode);

    if (sec.entry == PS_UNDEFINED)
    {
        printf("failed to find main entry point.\n");
        return PS_ERROR;
    }

    write(&sec, sizeof(TVMSection));

    for (Instruction ins : m_ins)
    {
        write8(ins.op);
        write8(ins.argc);
        write8(ins.flags);
        write16(ins.sizes);

        int i;
        for (i = 0; i < ins.argc; ++i)
        {
            if (ins.sizes & SizeFlags[i][0])
                write8((uint8_t)ins.argv[i]);
            else if (ins.sizes & SizeFlags[i][1])
                write16((uint16_t)ins.argv[i]);
            else if (ins.sizes & SizeFlags[i][2])
                write32((uint32_t)ins.argv[i]);
            else
                write64(ins.argv[i]);
        }
    }

    int pb = sec.align;
    while (pb--)
        write8(0);
    return m_sizeOfCode;
}

size_t BinaryWriter::writeSymbolSection(void)
{
    TVMSection sec = {};
    sec.size       = (uint32_t)m_sizeOfSym;
    sec.code       = TYPE_ID2('S', 'Y');
    sec.entry      = m_header.sym;
    sec.align      = getAlignment(m_sizeOfSym);

    write(&sec, sizeof(TVMSection));
    int pb = sec.align;
    while (pb--)
        write8(0);
    return m_sizeOfSym;
}

size_t BinaryWriter::writeStringSection(void)
{
    TVMSection sec = {};
    sec.size       = (uint32_t)m_sizeOfStr;
    sec.code       = TYPE_ID2('S', 'T');
    sec.entry      = m_header.str;
    sec.align      = getAlignment(m_sizeOfStr);
    write(&sec, sizeof(TVMSection));

    strvec_t::iterator it = m_orderedString.begin();
    while (it != m_orderedString.end())
    {
        const str_t& str = (*it++);
        write(str.c_str(), str.size());
        write8(0);
    }

    int pb = sec.align;
    while (pb--)
        write8(0);
    return m_sizeOfStr;
}


int BinaryWriter::writeSections()
{
    if (!m_fp)
        return PS_ERROR;

    size_t size;
    size = writeCodeSection();
    if (size != m_sizeOfCode)
        return PS_ERROR;

    size = writeDataSection();
    if (size != m_sizeOfData)
        return PS_ERROR;

    size = writeSymbolSection();
    if (size != m_sizeOfSym)
        return PS_ERROR;

    size = writeStringSection();
    if (size != m_sizeOfStr)
        return PS_ERROR;

    return PS_OK;
}

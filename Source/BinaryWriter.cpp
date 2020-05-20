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
#include <stdio.h>
#include <iostream>

using namespace std;

BinaryWriter::BinaryWriter() :
    m_fp(0),
    m_loc(0)
{
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

void BinaryWriter::write(void* v, size_t size)
{
    if (m_fp)
    {
        fwrite(v, size, 1, (FILE*)m_fp);
        m_loc = ftell((FILE*)m_fp);
    }
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

void BinaryWriter::open(const char* fname)
{
    if (m_fp)
        fclose((FILE*)m_fp);
    m_fp = fopen(fname, "wb");
    if (!m_fp)
        cout << "Failed to open '" << fname << "' for writing.\n";
}

size_t BinaryWriter::computeInstructionSize(const Instruction& ins)
{
    size_t size = 4;  // op, nr, flags, pad
    size += 8 * ins.argc; 
    return size;
}

size_t BinaryWriter::mapInstructions(void)
{
    uint64_t label = -1;
    uint64_t size  = 0;
    int64_t  insp  = 0;

    m_addrMap.clear();
    for (Instruction ins : m_ins)
    {
        if (ins.label != label)
        {
            label            = ins.label;
            m_addrMap[label] = insp;
        }
        size += computeInstructionSize(ins);
        ++insp;
    }
    return size;
}


int64_t BinaryWriter::findLabel(const std::string& name)
{
    if (!name.empty())
    {
        LabelMap::iterator it = m_labels.find(name);
        if (it != m_labels.end())
        {
            size_t                    idx  = it->second;
            IndexToPosition::iterator fidx = m_addrMap.find(idx);
            if (fidx != m_addrMap.end())
                return fidx->second;
        }
    }
    return -1;
}



void BinaryWriter::writeHeader()
{
    if (!m_fp)
        return;

    TVMHeader header = {};
    header.code  = TYPE_ID2('T', 'V');
    header.flags = 0;
    header.txt   = sizeof(TVMHeader);  // offsets to data
    header.dat   = 0;
    header.str   = 0;
    write(&header, sizeof(TVMHeader));
}

void BinaryWriter::writeSections()
{
    if (!m_fp)
        return;

    TVMSection sec = {};

    sec.size = (uint32_t)mapInstructions();
    if (sec.size <= 0)
        return;

    sec.code  = TYPE_ID2('C', 'S');
    sec.start = m_loc + sizeof(TVMSection);
    sec.entry = findLabel("main");
    
    if (sec.entry == -1)
    {
        cout << "Failed to find main.\n";
        return;
    }
    int64_t lookup;
    write(&sec, sizeof(TVMSection));

    for (Instruction ins : m_ins)
    {
        // look for changes in the labels then 
        // save the first position
        if (!ins.labelName.empty())
        {
            lookup = findLabel(ins.labelName);
            if (lookup != -1)
            {
                ins.argv[0] = lookup;
                ins.flags |= IF_ADDR;
            }
            else
                cout << "Unable to find index for " << ins.labelName << '\n';
        }

        write8(ins.op);
        write8(ins.argc);
        write8(ins.flags);
        write8(0);

        if (ins.argc > 0)
            write64(ins.argv[0]);
        if (ins.argc > 1)
            write64(ins.argv[1]);
        if (ins.argc > 2)
            write64(ins.argv[2]);
    }
}

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
#include <limits>

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
    {
        ins.sizes = calculateSizeFlag(ins);
        m_ins.push_back(ins);
    
    }
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
        fwrite(v, 1, size, (FILE*)m_fp);
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

size_t BinaryWriter::getLocation(void)
{
    m_loc = ftell((FILE*)m_fp);
    return m_loc;

}

uint16_t BinaryWriter::calculateSizeFlag(const Instruction& ins)
{
    uint16_t fl = 0;
    int i;
    for (i = 0; i < ins.argc; ++i)
    {
        if (ins.argv[i] < std::numeric_limits<uint8_t>().max())
            fl |= SizeFlags[i][0];
        else if (ins.argv[i] < std::numeric_limits<uint16_t>().max())
            fl |= SizeFlags[i][1];
        else if (ins.argv[i] < std::numeric_limits<uint32_t>().max())
            fl |= SizeFlags[i][2];
    }
    return fl;
}


size_t BinaryWriter::computeInstructionSize(const uint16_t& sizeBits, size_t argc)
{
    size_t size = 5;  // op, nr, flags, sizes

    size_t i;
    for (i = 0; i < argc; ++i)
    {
        if (sizeBits & SizeFlags[i][0])
            size += 1;
        else if (sizeBits & SizeFlags[i][1])
            size += 2;
        else if (sizeBits & SizeFlags[i][2])
            size += 4;
        else
            size += 8;
    }
    return size;
}

size_t BinaryWriter::mapInstructions(void)
{
    uint64_t label = PS_UNDEFINED;
    uint64_t size  = 0;
    int64_t  insp  = 0;

    m_addrMap.clear();

    for (Instruction ins : m_ins)
    {
        // look for changes in the label index then
        // save the index of the first change.
        if (ins.label != label)
        {
            label            = ins.label;
            m_addrMap[label] = insp;
        }

        size += computeInstructionSize(ins.sizes, ins.argc);
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
            IndexToPosition::iterator fidx = m_addrMap.find(it->second);
            if (fidx != m_addrMap.end())
                return fidx->second;
        }
    }

    return PS_UNDEFINED;
}


int BinaryWriter::writeHeader()
{
    if (!m_fp)
        return PS_ERROR;

    TVMHeader header = {};
    header.code  = TYPE_ID2('T', 'V');

    // TODO pre-compute section sizes 

    header.flags = 0;
    header.txt   = sizeof(TVMHeader);  // offsets to data
    header.dat   = 0;
    header.str   = 0;
    write(&header, sizeof(TVMHeader));

    return PS_OK;
}

int BinaryWriter::writeSections()
{
    if (!m_fp)
        return PS_ERROR;

    TVMSection sec = {};
    sec.size = (uint32_t)mapInstructions();
    sec.code  = TYPE_ID2('C', 'S');
    sec.start = getLocation() + sizeof(TVMSection);
    sec.entry = findLabel("main");

    if (sec.entry == PS_UNDEFINED)
    {
        printf("failed to find main entry point.\n");
        return PS_ERROR;
    }

    int64_t lookup;
    write(&sec, sizeof(TVMSection));

    for (Instruction ins : m_ins)
    {
        if (!ins.labelName.empty())
        {
            // modify the first argument so that it points
            // to the correct instruction index.
            lookup = findLabel(ins.labelName);
            if (lookup != PS_UNDEFINED)
            {
                ins.argv[0] = lookup;
                ins.flags |= IF_ADDR;
            }
            else
            {
                // this will have to change when external symbols are introduced.
                printf("Unable to find index for '%s'\n", ins.labelName.c_str());
                return PS_ERROR;
            }
        }

        write8(ins.op);
        write8(ins.argc);
        write8(ins.flags);
        write16(ins.sizes);

        int i;
        for (i=0; i<ins.argc; ++i)
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

    // TODO store values from .data sections

    return PS_OK;
}

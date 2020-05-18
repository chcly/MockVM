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
#include <iostream>
#include <stdio.h>
#include "BinaryWriter.h"

using namespace std;



BinaryWriter::BinaryWriter() :
    m_fp(0)
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


void BinaryWriter::open(const char* fname)
{
    if (m_fp)
        fclose((FILE*)m_fp);

    m_fp = fopen(fname, "wb");
    if (!m_fp)
        cout << "Failed to open '" << fname << "' for writing.\n";
}

void BinaryWriter::writeHeader()
{
    if (!m_fp)
        return;


    Header h = {};
    h.code = TYPE_ID4('T', 'V', 'M', '@');
    h.dat  = 0;
    h.str  = 0;
    h.txt  = sizeof(Header);
    fwrite(&h, 1, sizeof(Header), (FILE*)m_fp);
}

void BinaryWriter::writeSections()
{
    Section sec = {};
    sec.code = TYPE_ID2('@', 'C');
    sec.size = 0;
    sec.start  = sizeof(Header) + sizeof(Section);
    fwrite(&sec, 1, sizeof(Section), (FILE*)m_fp);

    for (Instruction ins : m_ins)
    {
        fwrite(&ins.op, 1, 1, (FILE*)m_fp);
        fwrite(&ins.argc, 1, 2, (FILE*)m_fp);
        fwrite(&ins.flags, 1, 4, (FILE*)m_fp);
        if (ins.argc > 0)
            fwrite(&ins.arg1, 1, 8, (FILE*)m_fp);
        if (ins.argc > 1)
            fwrite(&ins.arg1, 1, 8, (FILE*)m_fp);
    }
}

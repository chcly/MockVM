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

#include "BlockReader.h"
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <cassert>

BlockReader::BlockReader(const char *fname) :
    m_block(nullptr),
    m_fileLen(0),
    m_loc(0)
{
    open(fname);
}

BlockReader::BlockReader() :
    m_block(nullptr),
    m_fileLen(0),
    m_loc(0)
{
}

BlockReader::~BlockReader()
{
    if (m_block)
        delete[] m_block;
}

uint8_t BlockReader::next(void)
{
    uint8_t rc = 0;
    if (m_loc < m_fileLen)
    {
        rc = m_block[m_loc];
        m_loc++;
    }
    return rc;
}

uint8_t BlockReader::current(void)
{
    if (m_loc < m_fileLen)
        return m_block[m_loc];
    return 0;
}

size_t BlockReader::read(void *blk, size_t nr)
{
    if (eof() || nr == 0)
        return 0;

    if (m_loc + nr > m_fileLen)
        nr = m_fileLen - m_loc;

    memcpy(blk, &m_block[m_loc], nr);
    m_loc += nr;
    return nr;
}

void BlockReader::offset(int32_t nr)
{
    if (m_loc == 0 && nr < 0)
        return;

    int32_t lo = (int32_t)m_loc;
    lo += nr;
    if (lo < 0)
        lo = 0;
    if (lo > (int32_t)m_fileLen)
        lo = ((int32_t)m_fileLen) - 1;
    m_loc = lo;
}

void BlockReader::moveTo(size_t loc)
{
    if (loc < m_fileLen)
        m_loc = loc;
}

void BlockReader::open(const char *fname)
{
    if (fname)
    {
        FILE *fp = fopen(fname, "rb");
        if (fp)
        {
            fseek(fp, 0L, SEEK_END);
            m_fileLen = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            if (m_block)
                delete[] m_block;

            m_block = new uint8_t[m_fileLen + 1];
            fread(m_block, 1, m_fileLen, fp);
            m_block[m_fileLen] = 0;

            fclose(fp);
        }
        else
            puts("failed to open file.");
    }
    else
        puts("Invalid file name.");
}

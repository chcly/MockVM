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

BlockReader::BlockReader(const char *fname) :
    m_block(),
    m_fileLen(0),
    m_loc(0),
    m_fp(0),
    m_read(false)
{
    open(fname);
}

BlockReader::BlockReader() :
    m_block(),
    m_fileLen(0),
    m_loc(0),
    m_fp(0),
    m_read(false)
{
}

BlockReader::~BlockReader()
{
    if (m_fp)
        fclose((FILE *)m_fp);
}

uint8_t BlockReader::next(void)
{
    size_t remain = (m_loc + 1 % BLOCKSIZE);
    if (remain == 0 || !m_read)
        read();

    uint8_t rc = 0;
    if (m_loc < m_fileLen)
        rc = m_block[m_loc % BLOCKSIZE];
    m_loc++;
    return rc;
}

uint8_t BlockReader::current(void)
{
    if (!m_read)
        read();
    return m_block[m_loc % BLOCKSIZE];
}

void BlockReader::read(void *blk, size_t nr)
{
    if (m_fp)
    {
        size_t i = 0;
        while (i < nr && i < m_fileLen)
            ((uint8_t *)blk)[i++] = next();
    }
}

void BlockReader::offset(int32_t nr)
{
    if (m_fp && m_fileLen > 0)
    {
        if (((int32_t)m_loc) + nr < 0)
            m_loc = 0;

        m_loc += nr;

        if (m_loc > m_fileLen)
            m_loc = m_fileLen - 1;
    }
}

void BlockReader::moveTo(size_t loc)
{
    if (m_fp && loc < m_fileLen)
    {
        fseek((FILE *)m_fp, (long)loc, SEEK_SET);
        m_loc = loc;
    }
}

void BlockReader::read()
{
    if (m_fp)
    {
        m_read = true;

        size_t br = fread(m_block, 1, BLOCKSIZE, (FILE *)m_fp);
        if (br >= 0 && br <= BLOCKSIZE)
            m_block[br] = 0;
    }
}

void BlockReader::open(const char *fname)
{
    if (m_fp)
        fclose((FILE *)m_fp);

    if (fname)
    {
        m_fp = fopen(fname, "rb");
        if (m_fp)
        {
            fseek((FILE *)m_fp, 0L, SEEK_END);
            m_fileLen = ftell((FILE *)m_fp);
            fseek((FILE *)m_fp, 0L, SEEK_SET);
        }
        else
            puts("failed to open file.");
    }
    else
        puts("Invalid file name.");
}

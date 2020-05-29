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
#include "MemoryStream.h"
#include <memory.h>
#include <string.h>

MemoryStream::MemoryStream() :
    m_data(nullptr),
    m_size(0),
    m_capacity(0)
{
}

MemoryStream::~MemoryStream()
{
    clear();
}

void MemoryStream::clear(void)
{
    if (m_data)
        delete[] m_data;
    m_size     = 0;
    m_capacity = 0;
}

void MemoryStream::reserve(size_t nr)
{
    if (m_capacity < nr)
    {
        uint8_t* buf = new uint8_t[nr + 1];
        if (m_data != 0)
        {
            memcpy(buf, m_data, m_size);
            delete[] m_data;
        }
        m_data     = buf;
        m_capacity = nr;
    }
}

size_t MemoryStream::write(const void* src, size_t nr, bool pad)
{
    size_t al = 0;
    if (pad)
        al = findAllocLen(nr + 1);
    else
        al = findAllocLen(nr);

    if (al > 0)
        reserve(al);

    uint8_t* ptr = &m_data[m_size];
    memcpy(ptr, src, nr);
    m_size += nr;

    if (pad)
    {
        m_data[m_size] = 0;
        ++nr;
        ++m_size;
    }
    return nr;
}

size_t MemoryStream::findAllocLen(size_t nr)
{
    if (m_size + nr > m_capacity)
    {
        size_t lsize = m_size + nr;
        size_t over  = lsize - m_capacity;
        size_t r16   = over % 16;
        if (r16 > 0)
            over += 16 - r16;

        over += m_capacity;
        over += 256;
        return over;
    }
    return 0;
}

size_t MemoryStream::writeString(const char* src, size_t len)
{
    return write(src, len, true);
}

size_t MemoryStream::write8(uint8_t val)
{
    return write(&val, 1, false);
}

size_t MemoryStream::write16(uint16_t val)
{
    return write(&val, 2, false);
}

size_t MemoryStream::write32(uint32_t val)
{
    return write(&val, 4, false);
}

size_t MemoryStream::write64(uint64_t val)
{
    return write(&val, 8, false);
}

size_t MemoryStream::fill(size_t nr, uint8_t code)
{
    size_t al = findAllocLen(nr);
    if (al > 0)
        reserve(al);

    uint8_t* ptr = &m_data[m_size];
    memset(ptr, code, nr);
    m_size += nr;
    return nr;
}

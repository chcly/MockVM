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
#include "memory.h"
#include "string.h"



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
        m_data         = buf;
        m_data[m_size] = 0;
        m_capacity     = nr;
    }
}


size_t MemoryStream::write(const void* src, size_t nr, bool pad)
{
    m_capacity = m_size + nr + 1;
    if (pad)
        m_capacity++;

    uint8_t* ptr = new uint8_t[m_capacity];
    if (m_data)
    {
        memcpy(ptr, m_data, m_size);
        delete[] m_data;
    }
    m_data = ptr;
    memcpy(m_data + m_size, src, nr);
    m_size += nr;
    if (pad)
    {
        m_data[m_size++] = 0;
        nr++;
    }
    return nr;
}

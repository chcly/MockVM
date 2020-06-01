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
#ifndef _ArrayStack_h_
#define _ArrayStack_h_

#include <memory.h>
#include <stdint.h>
#include <string.h>

class ArrayStack
{
public:
    typedef uint64_t Data;

public:
    ArrayStack() :
        m_size(0),
        m_capacity(0),
        m_data(0)
    {
    }

    ~ArrayStack()
    {
        clear();
    }

    void clear(void)
    {
        if (m_data)
        {
            delete[] m_data;
            m_size     = 0;
            m_data     = nullptr;
            m_capacity = 0;
        }
    }

    void reserve(int32_t nr)
    {
        if (nr > m_capacity)
        {
            Data* dt = new Data[((size_t)nr) + 1];

            if (m_size > 0 && m_data != nullptr)
            {
                int32_t i;
                for (i = 0; i < m_size; i++)
                    dt[i] = m_data[i];
            }

            dt[nr]     = -1;
            m_capacity = nr;
            delete[] m_data;
            m_data = dt;
        }
    }

    void push(const uint64_t& v)
    {
        if (m_size + 1 > m_capacity)
            reserve(m_capacity == 0 ? 16 : m_capacity * 2);
        m_data[m_size] = v;
        ++m_size;
    }

    void pop(void)
    {
        if (m_size > 0)
            m_size--;
    }

    const uint64_t& peek(const size_t& idx) const
    {
        int32_t iv = (m_size - 1) - (int32_t)idx;
        if (iv >= 0)
            return m_data[iv];
        return m_data[m_capacity];
    }

    uint64_t& peek(const size_t& idx)
    {
        int32_t iv = (m_size - 1) - (int32_t)idx;
        if (iv >= 0)
            return m_data[iv];
        return m_data[m_capacity];
    }

    const uint64_t& top(void) const
    {
        return peek(0);
    }

    inline bool empty(void) const
    {
        return m_size <= 0;
    }

    inline const int32_t& size(void) const
    {
        return m_size;
    }

    inline const int32_t& capacity(void) const
    {
        return m_capacity;
    }

private:
    int32_t m_size;
    int32_t m_capacity;
    Data*   m_data;
};

#endif  //_ArrayStack_h_

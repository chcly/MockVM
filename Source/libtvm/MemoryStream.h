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
#ifndef _MemoryStream_h_
#define _MemoryStream_h_

#include "Declarations.h"


class MemoryStream
{
public:
    MemoryStream();
    ~MemoryStream();

    void   clear(void);
    size_t writeString(const char* src, size_t len);
    size_t write8(uint8_t val);
    size_t write16(uint16_t val);
    size_t write32(uint32_t val);
    size_t write64(uint64_t val);
    size_t fill(size_t nr, uint8_t code);

    void reserve(size_t cap);

    inline size_t size(void) const
    {
        return m_size;
    }
    
    inline size_t capacity(void) const
    {
        return m_capacity;
    }

    inline uint8_t* ptr()
    {
        return m_data;
    }

    inline const uint8_t* ptr() const
    {
        return m_data;
    }

private:

    size_t findAllocLen(size_t nr);
    size_t write(const void *ptr, size_t nr, bool pad);


    size_t m_size;
    size_t m_capacity;

    uint8_t* m_data;
};

#endif  //_MemoryStream_h_
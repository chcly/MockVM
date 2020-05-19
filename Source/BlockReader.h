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
#ifndef _BlockReader_h_
#define _BlockReader_h_

#include <stdint.h>


class BlockReader
{
public:
    static const size_t BLOCKSIZE = 2048;
    typedef uint8_t     Block[BLOCKSIZE + 1];

private:
    Block  m_block;
    size_t m_fileLen;
    size_t m_loc;
    void * m_fp;

    void read(void);

public:
    BlockReader(const char *fname);
    BlockReader();
    ~BlockReader();

    void    open(const char *fname);
    uint8_t next(void);
    void    read(void *blk, size_t nr);
    void    offset(size_t nr);
    void    moveTo(size_t loc);
        
    inline bool eof(void)
    {
        return m_fp == nullptr || m_loc > m_fileLen;
    }

    inline size_t tell(void)
    {
        return m_loc;
    }

    inline size_t size(void)
    {
        return m_fileLen;
    }
};

#endif//_BlockReader_h_

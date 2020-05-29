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
#include "Catch2.h"
#include "MemoryStream.h"


// Basic reserve test
TEST_CASE("Memory1")
{
    MemoryStream ms;
    ms.reserve(256);

    EXPECT_EQ(ms.capacity(), 256);
    EXPECT_EQ(ms.size(), 0);
    EXPECT_NE(ms.ptr(), nullptr);

    // reserving a smaller size should keep
    // the same amount of memory.
    ms.reserve(128);
    memset(ms.ptr(), 0xFF, ms.capacity());

    EXPECT_EQ(ms.capacity(), 256);
    EXPECT_EQ(ms.size(), 0);
    EXPECT_NE(ms.ptr(), nullptr);
}


// Testing expansion.
TEST_CASE("Memory2")
{
    MemoryStream ms;
    for (int  i=0; i< 272; ++i)
        ms.write8('A');

    EXPECT_EQ(272, ms.capacity());
    ms.write8('E'); // cause an expansion
    EXPECT_EQ(544, ms.capacity());

    char *tp = new char[272];
    memset(tp, 'A', 272);

    bool pass = memcmp(ms.ptr(), tp, 272) == 0;
    EXPECT_TRUE(pass);
    delete[] tp;

    for (int i = 272; i < 543; ++i)
        ms.write8('Z');

    EXPECT_EQ(544, ms.capacity());
    ms.write8('E');  // cause an expansion
    EXPECT_EQ(816, ms.capacity());

    char *blk = new char[547];

    tp = blk;
    memset(tp, 'A', 272);
    tp += 272;
    *tp++ = 'E';
    memset(tp, 'Z', 272);
    tp += 272;
    *tp++ = 'E';

    pass = memcmp(ms.ptr(), blk, 544) == 0;
    EXPECT_TRUE(pass);
    delete[] blk;
}



// Testing return values
TEST_CASE("Memory3")
{
    MemoryStream ms;

    bool   pass = false;
    size_t of = 0, sz = 0;
    int    i;
    for (i=0; i<7; ++i)
    {

        sz = ms.size();
        of += ms.writeString("HelloWorld", 10);
        EXPECT_EQ(of, sz + 11);  // plus the null terminator.
   
        sz = ms.size();
        of += ms.write8('A');
        EXPECT_EQ(of, sz + 1);

        sz = ms.size();
        of += ms.write16(0x012F);
        EXPECT_EQ(of, sz + 2);
        
        sz = ms.size();
        of += ms.write32(0xF0EAD01F);
        EXPECT_EQ(of, sz + 4);

        sz = ms.size();
        of += ms.write64(0x0123456789ABCDEF);
        EXPECT_EQ(of, sz + 8);
    }

    EXPECT_EQ(272, ms.capacity());


    uint8_t *ptr = ms.ptr();

    for (i = 0; i < 7; ++i)
    {
        char *cp = (char*)ptr;

        pass = strncmp(cp, "HelloWorld", 10) == 0;
        EXPECT_TRUE(pass);
        ptr += 11;

        EXPECT_EQ(*ptr, 'A');
        ptr += 1;

        uint16_t *v16 = (uint16_t*)ptr;
        EXPECT_EQ(*v16, 0x012F);
        ptr += 2;

        uint32_t *v32 = (uint32_t *)ptr;
        EXPECT_EQ(*v32, 0xF0EAD01F);
        ptr += 4;

        uint64_t *v64 = (uint64_t *)ptr;
        EXPECT_EQ(*v64, 0x0123456789ABCDEF);
        ptr += 8;
    }

}

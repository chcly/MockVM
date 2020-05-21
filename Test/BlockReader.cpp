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
#include "BlockReader.h"

const std::string KeywordsFile = std::string(TestDirectory) + "/Keywords.asm";

TEST_CASE("BlockReder_0")
{
    BlockReader r;
    r.open(KeywordsFile.c_str());
    EXPECT_FALSE(r.eof());

    size_t size = r.size(), i;

    uint8_t *tmp = new uint8_t[size + 1];



    for (i =0; i<size && !r.eof(); ++i)
    {
        // current returns the buffer at the internal position
        uint8_t tch = r.current();

        // current returns the buffer at the internal position
        // then advances the position by one
        uint8_t ch = r.next();
        tmp[i] = ch;
        EXPECT_EQ(ch, tch);

        r.offset(-1);
        EXPECT_EQ(ch, r.current());
        r.offset(1);
    }

    EXPECT_EQ(memcmp(tmp, r.ptr(), size), 0);
    delete[] tmp;
}

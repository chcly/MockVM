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
#include "Parser.h"
#include "Catch2.h"


TEST_CASE("Scan1")
{
    const std::string Scan1 = std::string(TestDirectory) + "/Scan/Scan1.asm";

    Parser            p;
    int sr  = p.open(Scan1.c_str());
    EXPECT_EQ(sr, PS_OK);


    Token tok = {};
    sr = p.scan(tok);
    EXPECT_NE(sr,       PS_ERROR);
    EXPECT_EQ(tok.type, TOK_IDENTIFIER);
    EXPECT_EQ(tok.value, "abc");
    sr = p.scan(tok);
    EXPECT_EQ(sr, PS_EOF);
}

TEST_CASE("Scan2")
{
    const std::string Scan1 = std::string(TestDirectory) + "/Scan/Scan2.asm";

    Parser p;
    int    sr = p.open(Scan1.c_str());
    EXPECT_EQ(sr, PS_OK);

    Token tok;
    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_LABEL);
    EXPECT_EQ(tok.value, "main");

    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_LABEL);
    EXPECT_EQ(tok.value, "abc");
    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_LABEL);
    EXPECT_EQ(tok.value, "def");

    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_LABEL);
    EXPECT_EQ(tok.value, "ghi");
    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_OPCODE);
    EXPECT_EQ(tok.op, OP_MOV);

    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_REGISTER);
    EXPECT_EQ(tok.reg, 0);

    sr = p.scan(tok);
    EXPECT_EQ(tok.type, TOK_DIGIT);
    EXPECT_EQ(tok.ival.x, 0);

    sr = p.scan(tok);
    EXPECT_EQ(tok.type, PS_OK);
}

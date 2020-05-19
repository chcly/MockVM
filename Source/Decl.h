/*
-------------------------------------------------------------------------------
    Copyright (c) 20120 Charles Carley.

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
#ifndef _Decl_h_
#define _Decl_h_



enum TokenCode
{
    TOK_MNEMONIC,  // keyword
    TOK_REGISTER,
    TOK_IDENTIFIER,
    TOK_COLON,
    TOK_COMMA,
    TOK_PERIOD,
    TOK_HEX,
    TOK_BINARY,
    TOK_DIGIT,
    TOK_LABEL,
    TOK_SECTION,
    TOK_EOL,
};

enum CharType
{
    CT_LALPHA,
    CT_UALPHA,
    CT_DIGIT,
    CT_WS,
    CT_NL,
    CT_COMMA,
    CT_PERIOD,
    CT_COLON,
    CT_SQUOTE,
    CT_DQUOTE,
    CT_LPARAN,
    CT_RPARAN,
    CT_LBRACE,
    CT_RBRACE,
    CT_HASH,
    CT_NULL,
    CT_MAX,
};


enum ParserState
{
    ST_INITIAL = 0,
    ST_READ_ID,
    ST_DIGIT,
    ST_EXIT,
    ST_ERROR,
    ST_CONTINUE,
    ST_SECTION,
    ST_MAX,
};

enum Opcode
{
    OP_RET  = 0x90,
    OP_MOV  = 0x0F,  // mov dst, src
    OP_CALL = 0x10,
    OP_TRACE,
};

enum RegisterCode
{
    REG_X0 = 0x00,
    REG_X1 = 0x01,
    REG_X2 = 0x02,
    REG_X3 = 0x03,
    REG_X4 = 0x04,
    REG_X5 = 0x05,
    REG_X6 = 0x06,
    REG_X7 = 0x07,
    REG_X8 = 0x08,
    REG_X9 = 0x09,
};


enum Actions
{
    AC_IG = -3,  // ignore
    AC_0L = -2,  // return TOK_EOL
    AC_0E = -1,  // error
    AC_00,       // Add char to token
    AC_01,       // test keywords, if keyword return token TOK_MNEMONIC, else TOK_IDENTIFIER
    AC_02,       // Add char to token, goto STATE ST_READ_ID
    AC_03,       // goto STATE ST_INITIAL, return TOK_LABEL
    AC_04,       // goto STATE ST_INITIAL, return TOK_IDENTIFIER
    AC_05,       // goto STATE ST_INITIAL continue
    AC_06,       // goto STATE ST_DIGIT continue
    AC_07,       // goto STATE ST_CONTINUE return TOK_DIGIT
    AC_SC,       // goto STATE ST_SECTION
    AC_DS,       // goto STATE ST_INITIAL return TOK_SECTION
};



typedef char Keyword[7];
struct KeywordMap
{
    Keyword       word;
    unsigned char op;
};


#endif // _Decl_h_
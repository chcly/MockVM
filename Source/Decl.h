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
    TOK_MAX,
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
    CT_ADD,
    CT_SUB,
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
    OP_BEG,
    OP_RET,
    OP_MOV,   // mov r(n), src
    OP_CALL,  // call address
    OP_INC,   // inc, r(n)
    OP_DEC,   // dec, r(n)
    OP_CMP,   // cmp, r(n), src
    OP_JMP,   // jump
    OP_JEQ,   // jump ==
    OP_JNE,   // jump !
    OP_JLT,   // jump <
    OP_JGT,   // jump >
    OP_JLE,   // jump <=
    OP_JGE,   // jump >=
              // ---- debugging ----
    OP_PRG,   // print register
    OP_PRGI,  // print all registers
    OP_MAX
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


enum ArgType
{
    AT_REG,
    AT_LIT,
    AT_ADDR,
    AT_REGLIT,
    AT_NULL,
};


typedef char Keyword[7];


struct KeywordMap
{
    Keyword       word;
    unsigned char op;
    unsigned char narg;
    unsigned char type_arg1;
    unsigned char type_arg2;
    unsigned char type_arg3;
};


#endif // _Decl_h_
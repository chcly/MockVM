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
#ifndef _Parser_h_
#define _Parser_h_

#include <stdint.h>
#include <string>
#include <stack>

#include "BlockReader.h"
#include "Decl.h"

struct Token
{
    uint8_t     op;
    uint8_t     reg;
    int64_t     ival;
    std::string value;
};


class Parser
{
public:
    typedef int32_t StateTable[ST_MAX][CT_MAX];
    typedef int32_t (Parser::*Action)(uint8_t ch);

    typedef std::stack<Token> TokenStack;

    const static Parser::Action Actions[];
    const static size_t         ActionCount;
    const static StateTable     States;

private:
    BlockReader m_reader;
    int32_t     m_state;
    std::string m_curString;
    int64_t     m_ival;
    uint8_t     m_op;
    uint8_t     m_register;
    TokenStack  m_tokens;

    int32_t scan(void);
    int32_t getType(uint8_t ct);
    void    clearState(void);


public:
    Parser();
    ~Parser();

    void parse(const char* fname);


private:
    int32_t ActionIdx00(uint8_t ch);
    int32_t ActionIdx01(uint8_t ch);
    int32_t ActionIdx02(uint8_t ch);
    int32_t ActionIdx03(uint8_t ch);
    int32_t ActionIdx04(uint8_t ch);
    int32_t ActionIdx05(uint8_t ch);
    int32_t ActionIdx06(uint8_t ch);
    int32_t ActionIdx07(uint8_t ch);
    int32_t ActionIdxSC(uint8_t ch);
    int32_t ActionIdxDC(uint8_t ch);
};

#endif  //_Parser_h_

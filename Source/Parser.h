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
#include <vector>
#include <unordered_map>


#include "Instruction.h"
#include "BlockReader.h"
#include "Declarations.h"

struct Token
{
    uint8_t     op;
    uint8_t     reg;
    Register    ival;
    int32_t     type;
    std::string value;
    int32_t     index;
};

enum ParseResult
{
    PS_ERROR=-10,
    PS_OK,
};


class Parser
{
public:
    typedef int32_t StateTable[ST_MAX][CT_MAX];
    typedef int32_t (Parser::*Action)(uint8_t ch);
    
    

    typedef std::stack<Token>                       TokenStack;
    typedef std::unordered_map<std::string, size_t> LabelMap;
    typedef std::vector<Instruction>                Instructions;

    const static Parser::Action Actions[];
    const static size_t         ActionCount;
    const static StateTable     States;
    const static KeywordMap     KeywordTable[];
    const static size_t         KeywordTableSize;


private:

    BlockReader  m_reader;
    int32_t      m_state, m_section, m_label, m_lineNo;
    std::string  m_curString;
    int64_t      m_ival;
    uint8_t      m_op;
    uint8_t      m_register;
    TokenStack   m_tokens;
    LabelMap     m_labels;
    Instructions m_instructions;
    std::string  m_fname;

    int32_t scan(void);
    int32_t getType(uint8_t ct);
    void    clearState(void);
    void    error(const char* fmt, ...);

    int32_t           getSection(const std::string& val);
    int32_t           getKeywordIndex(const uint8_t& val);
    const KeywordMap& getKeyword(const int32_t& val);
    
    Token             scanNextToken(void);
    Token             getLastToken(void);
    int32_t           handleArgument(Instruction&      ins,
                                     const KeywordMap& kwd,
                                     const Token&      tok,
                                     const int32_t     idx);


public:
    Parser();
    ~Parser();

    int32_t parse(const char* fname);

    inline const Instructions& getInstructions()
    {
        return m_instructions;
    }

    inline const LabelMap& getLabels()
    {
        return m_labels;
    }

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

    int32_t handleOpCode(const Token& tok);
    int32_t TokenSECTION(void);
    int32_t TokenLABEL(void);
};

#endif  //_Parser_h_

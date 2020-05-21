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
#include "BlockReader.h"
#include "Declarations.h"



class Parser
{
public:
    typedef std::unordered_map<std::string, size_t> LabelMap;
    typedef std::vector<Instruction>                Instructions;

    const static KeywordMap     KeywordTable[];
    const static size_t         KeywordTableSize;

private:
    BlockReader  m_reader;
    int32_t      m_state;
    int32_t      m_section;
    int32_t      m_label;
    int32_t      m_lineNo;
    LabelMap     m_labels;
    Instructions m_instructions;
    std::string  m_fname;

    int32_t scan(Token &tok);
    void    error(const char* fmt, ...);

    int32_t           getSection(const std::string& val);
    int32_t           getKeywordIndex(const uint8_t& val);
    const KeywordMap& getKeyword(const int32_t& val);
    
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
    int32_t handleOpCode(const Token& tok);
    int32_t handleSection(const Token& tok);
    int32_t handleLabel(const Token& dest);

    void    countNewLine(uint8_t ch);
    uint8_t scanEol(void);
    int32_t handleInitialState(Token& dest);
    int32_t handleIdState(Token& dest);
    int32_t handleDigitState(Token& dest);
    int32_t handleSectionState(Token& dest);
};

#endif  //_Parser_h_

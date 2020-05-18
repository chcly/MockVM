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
#include <iostream>
#include "Decl.h"
#include "Instruction.h"

using namespace std;

#define CONTINUE -1

Parser::Parser() :
    m_state(0),
    m_curString(),
    m_ival(0),
    m_op(0),
    m_register(0)
{
}

Parser::~Parser()
{
}

void Parser::clearState(void)
{
    m_curString.clear();
    m_op       = 0;
    m_ival     = 0;
    m_register = 0;
}

void Parser::parse(const char* fname)
{
    m_reader.open(fname);
    if (!m_reader.eof())
    {
        m_state = ST_INITIAL;
        int32_t sr;
        m_section = -1;
        m_label   = -1;

        while (!m_reader.eof())
        {
            sr = scan();
            if (sr == TOK_SECTION)
            {
                m_section = getSection(m_tokens.top().value);
                if (m_section == -1)
                    cout << "Unknown section: " << m_tokens.top().value;
                m_tokens.pop();
            }
            else if (sr == TOK_LABEL)
            {
                const Token& t = m_tokens.top();

                // use position in buffer for its mapping.
                m_labels[t.value] = m_instructions.size();

                m_tokens.pop();
            }
            else if (sr == TOK_MNEMONIC)
            {
                const Token &t = m_tokens.top();
                m_tokens.pop();
                switch (t.op)
                {
                case OP_MOV:
                    RuleMOV();
                    break;
                case OP_RET:
                    RuleRET();
                    break;
                default:
                    break;
                }
            }
            else if (sr == TOK_REGISTER)
            {
                Token t = m_tokens.top();
                cout << (int)t.reg << '\n';
                m_tokens.pop();
            }
            else if (sr == TOK_DIGIT)
            {
                Token t = m_tokens.top();
                cout << (int)t.ival << '\n';
                m_tokens.pop();
            }
        }
    }
    else
        cout << "Failed to load '" << fname << "'\n";
}

int32_t Parser::scan(void)
{
    while (!m_reader.eof())
    {
        uint8_t ch = m_reader.next();
        int32_t tk = getType(ch);
        int32_t ac = States[m_state][tk];

        if (ac >= 0 && ac < ActionCount)
        {
            if (Actions[ac] != nullptr)
            {
                int32_t rc = (this->*Actions[ac])(ch);
                if (rc != -1)
                {
                    m_tokens.push({m_op, m_register, m_ival, m_curString});
                    return rc;
                }
            }
        }
        else if (ac == AC_0E)
            cout << "Unknown character parsed '" << ch << "'\n";
    }
    return -1;
}


void Parser::RuleMOV()
{
    // The mnemonic has been parsed at this point,
    // so figure out the rules.
    
    int32_t tok;
    tok = scan();
    if (tok != TOK_REGISTER)
    {
        cout << "Expected first argument to mov to be a register\n";
        return;
    }

    const Token& reg = m_tokens.top();
    m_tokens.pop();

    Instruction ins = {};

    ins.op   = OP_MOV;
    ins.arg1 = reg.reg;
    ins.flags |= IF_DREG;


    tok = scan();

    const Token& res = m_tokens.top();
    m_tokens.pop();

    if (tok == TOK_REGISTER)
    {
        ins.arg2 = res.reg;
        ins.flags |= IF_SREG;
    }
    else if (tok == TOK_DIGIT)
    {
        ins.arg2 = res.ival;
        ins.flags |= IF_SLIT;
    }
    else
    {
        cout << "Unknown second argument to mov.\n";
        return;
    }


    ins.argc = 2;
    ins.label = m_label;
    m_instructions.push_back(ins);
}


void Parser::RuleRET(void)
{
    Instruction ins = {};
    ins.op          = OP_RET;
    ins.argc        = 0;
    ins.label       = m_label;
    m_instructions.push_back(ins);
}


int32_t Parser::getSection(const std::string& val)
{
    if (val == "data")
        return SEC_DAT;
    else if (val == "text")
        return SEC_TXT;
    return -1;
}

int32_t Parser::ActionIdx00(uint8_t ch)
{
    // Append to the current buffer
    m_curString.push_back(ch);
    return CONTINUE;
}

int32_t Parser::ActionIdx01(uint8_t ch)
{
    if (!m_curString.empty())
    {
        m_state = ST_CONTINUE;

        if (m_curString.size() == 2 &&
            m_curString[1] >= '0' &&
            m_curString[1] <= '9')
        {
            if (m_curString[0] == 'x')
            {
                m_register = m_curString[1] - '0';
                return TOK_REGISTER;
            }
        }
        size_t i = 0;
        for (i = 0; i < KeywordTableSize; ++i)
        {
            if (strncmp(KeywordTable[i].word, m_curString.c_str(), 6) == 0)
            {
                m_curString.clear();
                m_op = KeywordTable[i].op;
                return TOK_MNEMONIC;
            }
        }
        return TOK_IDENTIFIER;
    }
    return CONTINUE;
}

int32_t Parser::ActionIdx02(uint8_t ch)
{
    m_state = ST_READ_ID;
    clearState();
    m_curString.push_back(ch);
    return CONTINUE;
}

int32_t Parser::ActionIdx03(uint8_t ch)
{
    m_state = ST_INITIAL;
    return TOK_LABEL;
}

int32_t Parser::ActionIdx04(uint8_t ch)
{
    m_state = ST_INITIAL;
    return TOK_IDENTIFIER;
}

int32_t Parser::ActionIdx05(uint8_t ch)
{
    m_state = ST_INITIAL;
    clearState();
    return CONTINUE;
}

int32_t Parser::ActionIdx06(uint8_t ch)
{
    m_state = ST_DIGIT;
    clearState();
    m_curString.push_back(ch);
    return CONTINUE;
}

int32_t Parser::ActionIdx07(uint8_t ch)
{
    m_state = ST_CONTINUE;
    m_ival  = strtoull(m_curString.c_str(), nullptr, 10);
    return TOK_DIGIT;
}

int32_t Parser::ActionIdxSC(uint8_t ch)
{
    m_state = ST_SECTION;
    clearState();
    return CONTINUE;
}

int32_t Parser::ActionIdxDC(uint8_t ch)
{
    m_state = ST_INITIAL;
    return TOK_SECTION;
}

const Parser::Action Parser::Actions[] = {
    &Parser::ActionIdx00,
    &Parser::ActionIdx01,
    &Parser::ActionIdx02,
    &Parser::ActionIdx03,
    &Parser::ActionIdx04,
    &Parser::ActionIdx05,
    &Parser::ActionIdx06,
    &Parser::ActionIdx07,
    &Parser::ActionIdxSC,
    &Parser::ActionIdxDC,
    nullptr,
};

const size_t Parser::ActionCount = sizeof(Actions) / sizeof(Actions[0]);

const Parser::StateTable Parser::States = {
    //[a-z] [A-Z]  [0-9]   ' '    '\n'    ','    '.'    ':'    '''    '"'    '('    ')'    '['    ']'    '#'   \0
    {AC_02, AC_02, AC_06, AC_01, AC_05, AC_0E, AC_SC, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_01},  // ST_INITIAL
    {AC_00, AC_00, AC_00, AC_04, AC_01, AC_01, AC_IG, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_READ_ID
    {AC_0E, AC_0E, AC_00, AC_07, AC_07, AC_0E, AC_IG, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_DIGIT
    {AC_0E, AC_0E, AC_0E, AC_0E, AC_0L, AC_0E, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_EXIT
    {AC_0E, AC_0E, AC_0E, AC_0E, AC_0L, AC_0E, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_ERROR
    {AC_0E, AC_0E, AC_0E, AC_05, AC_05, AC_05, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_CONTINUE
    {AC_00, AC_0E, AC_0E, AC_DS, AC_DS, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_SECTION
};


int32_t Parser::getType(uint8_t ct)
{
    int32_t rc = CharType::CT_NULL;
    if (ct >= 'a' && ct <= 'z')
        rc = CharType::CT_LALPHA;
    else if (ct >= 'A' && ct <= 'Z')
        rc = CharType::CT_UALPHA;
    else if (ct >= '0' && ct <= '9')
        rc = CharType::CT_DIGIT;
    else
    {
        switch (ct)
        {
        case ' ':
        case '\t':
            rc = CharType::CT_WS;
            break;
        case '\n':
        case '\r':
            rc = CharType::CT_NL;
            break;
        case '\'':
            rc = CharType::CT_SQUOTE;
            break;
        case '"':
            rc = CharType::CT_DQUOTE;
            break;
        case '(':
            rc = CharType::CT_LPARAN;
            break;
        case ')':
            rc = CharType::CT_RPARAN;
            break;
        case '[':
            rc = CharType::CT_LBRACE;
            break;
        case ']':
            rc = CharType::CT_RBRACE;
            break;
        case ',':
            rc = CharType::CT_COMMA;
            break;
        case '.':
            rc = CharType::CT_PERIOD;
            break;
        case ':':
            rc = CharType::CT_COLON;
            break;
        case '#':
            rc = CharType::CT_HASH;
            break;
        default:
            rc = CharType::CT_NULL;
            break;
        }
    }
    return rc;
}


const KeywordMap Parser::KeywordTable[] = {
    {"mov\0", OP_MOV},
    {"ret\0", OP_RET},
};

const size_t Parser::KeywordTableSize = 
    sizeof(Parser::KeywordTable) / sizeof(KeywordMap);

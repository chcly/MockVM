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
#include "Instruction.h"

using namespace std;

#define CONTINUE -1

const string     Blank            = "";
const string     UndefinedToken   = "Undefined token:";
const string     UnknownArg       = "Missing Argument:";
const string     EmptyStack       = "Empty token stack";
const string     UndefinedChar    = "Unknown character parsed";
const string     Undefinedsection = "Undefined section type";
const Token      InvalidToken     = {0xFF, 0xFF, {0}, TOK_MAX, Blank};
const KeywordMap NullKeyword      = {{}, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

Parser::Parser() :
    m_state(0),
    m_section(-1),
    m_label(-1),
    m_lineNo(1),
    m_curString(),
    m_ival(0),
    m_op(0),
    m_register(0),
    m_tokens(),
    m_labels(),
    m_instructions()
{
}

Parser::~Parser()
{
}

int32_t Parser::parse(const char* fname)
{
    m_reader.open(fname);
    if (!m_reader.eof())
    {
        m_state = ST_INITIAL;
        int32_t sr, rc;
        m_section = -1;
        m_label   = 0;

        while (!m_reader.eof())
        {
            sr = scan();
            switch (sr)
            {
            case TOK_MNEMONIC:
                rc = handleOpCode(getLastToken());
                break;
            case TOK_SECTION:
                rc = TokenSECTION();
                break;
            case TOK_LABEL:
                rc = TokenLABEL();
                break;
            case PS_OK:
                return PS_OK;
            default:
                rc = PS_ERROR;
                break;
            }

            if (rc != PS_OK)
                return PS_ERROR;
        }
    }
    else
    {
        cout << "Failed to load '" << fname << "'\n";
        return PS_ERROR;
    }
    return PS_OK;
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
                    Token tok  = {};
                    tok.op     = m_op;
                    tok.reg    = m_register;
                    tok.ival.x = m_ival;
                    tok.value  = m_curString;
                    tok.type   = rc;
                    tok.index  = getKeywordIndex(m_op);
                    m_tokens.push(tok);
                    return rc;
                }
            }
        }
        else if (ac == -1)
        {
            cout << UndefinedChar << ' '
                 << '\'' << ch << "'\n";
            return PS_ERROR;
        }
    }
    return PS_OK;
}

Token Parser::getLastToken(void)
{
    if (!m_tokens.empty())
    {
        Token rval = m_tokens.top();
        m_tokens.pop();
        return rval;
    }
    return InvalidToken;
}

Token Parser::scanNextToken(void)
{
    scan();
    if (!m_tokens.empty())
    {
        Token reg = m_tokens.top();
        m_tokens.pop();
        return reg;
    }
    return InvalidToken;
}

int32_t Parser::TokenSECTION(void)
{
    const Token& tok = getLastToken();

    m_section = getSection(tok.value);
    if (m_section == -1)
    {
        cout << Undefinedsection << ' '
             << tok.value << '\n';
        return PS_ERROR;
    }
    return PS_OK;
}

int32_t Parser::TokenLABEL(void)
{
    const Token& label = getLastToken();
    if (label.type == TOK_LABEL)
    {
        if (m_labels.find(label.value) != m_labels.end())
        {
            cout << "error duplicate label '" << label.value << "'\n";
            return PS_ERROR;
        }

        // This will be resolved after all text has been parsed.
        // For now this just needs to map to a unique index.
        m_labels[label.value] = ++m_label;
    }
    else
    {
        cout << "internal error, token type does not match the scanned token\n";
        return PS_ERROR;
    }
    return PS_OK;
}


int32_t Parser::handleOpCode(const Token& tok)
{
    const KeywordMap& kwd = getKeyword(tok.index);
    if (kwd.op != -1)
    {
        Instruction ins = {};

        ins.op    = kwd.op;
        ins.argc  = kwd.narg;
        ins.label = m_label;

        if (ins.argc > 0)
        {
            const Token& arg1 = scanNextToken();
            if (kwd.type_arg1 == AT_REG)
            {
                if (arg1.type != TOK_REGISTER)
                {
                    cout << "Expected operand one, for " << kwd.word << " to be a ";
                    cout << "register. found type " << arg1.type;
                    cout << '\n';
                    return PS_ERROR;
                }

                ins.arg1 = arg1.reg;
                ins.flags |= IF_DREG;
            }
            else if (kwd.type_arg1 == AT_LIT)
            {
                if (arg1.type != TOK_DIGIT)
                {
                    cout << "Expected operand one, for " << kwd.word << " to be a ";
                    cout << "literal. found type " << arg1.type;
                    cout << '\n';
                    return PS_ERROR;
                }

                ins.arg1 = arg1.ival.x;
                ins.flags |= IF_DLIT;
            }
            else if (kwd.type_arg1 == AT_REGLIT)
            {
                if (arg1.type == TOK_DIGIT)
                {
                    ins.arg2 = arg1.ival.x;
                    ins.flags |= IF_DLIT;
                }
                else if (arg1.type == TOK_REGISTER)
                {
                    ins.arg1 = arg1.reg;
                    ins.flags |= IF_DREG;
                }
                else
                {
                    cout << "Expected operand two, for " << kwd.word << " to be either a ";
                    cout << "literal or a register. found type " << arg1.type;
                    cout << '\n';
                    return PS_ERROR;
                }
            }
            else if (kwd.type_arg1 == AT_ADDR)
            {
                // save this now so it can be resolved after all
                // labels have been stored
                ins.labelName = arg1.value;
            }
            else
            {
                cout << "Unknown first operand for " << kwd.word << '\n';
                return PS_ERROR;
            }
        }

        if (ins.argc > 1)
        {
            const Token& arg2 = scanNextToken();
            if (kwd.type_arg2 == AT_REG)
            {
                if (arg2.type != TOK_REGISTER)
                {
                    cout << "Expected operand two, for " << kwd.word << " to be a ";
                    cout << "register. found type " << arg2.type;
                    cout << '\n';
                    return PS_ERROR;
                }

                ins.arg2 = arg2.reg;
                ins.flags |= IF_SREG;
            }
            else if (kwd.type_arg2 == AT_LIT)
            {
                if (arg2.type != TOK_DIGIT)
                {
                    cout << "Expected operand one, for " << kwd.word << " to be a ";
                    cout << "literal. found type " << arg2.type;
                    cout << '\n';
                    return PS_ERROR;
                }

                ins.arg2 = arg2.ival.x;
                ins.flags |= IF_SLIT;
            }

            else if (kwd.type_arg2 == AT_REGLIT)
            {
                if (arg2.type == TOK_DIGIT)
                {
                    ins.arg2 = arg2.ival.x;
                    ins.flags |= IF_SLIT;
                }
                else if (arg2.type == TOK_REGISTER)
                {
                    ins.arg2 = arg2.reg;
                    ins.flags |= IF_SREG;
                }
                else
                {
                    cout << "Expected operand two, for " << kwd.word << " to be either a ";
                    cout << "literal or a register. found type " << arg2.type;
                    cout << '\n';
                    return PS_ERROR;
                }
            }
            else if (kwd.type_arg1 == AT_ADDR)
            {
                // save this now so it can be resolved after all
                // labels have been stored
                ins.labelName = arg2.value;
            }
            else
            {
                cout << "Unknown second operand for " << kwd.word << '\n';
                return PS_ERROR;
            }
        }

        m_instructions.push_back(ins);
        return PS_OK;
    }
    return PS_ERROR;
}

void Parser::clearState(void)
{
    m_curString.clear();
    m_op       = 0;
    m_ival     = 0;
    m_register = 0;
}

int32_t Parser::getSection(const std::string& val)
{
    if (val == "data")
        return SEC_DAT;
    else if (val == "text")
        return SEC_TXT;
    return -1;
}

int32_t Parser::getKeywordIndex(const uint8_t& val)
{
    if (val == 0)
        return -1;

    int32_t i;
    for (i = 0; i < KeywordTableSize; ++i)
    {
        if (KeywordTable[i].op == val)
            return i;
    }
    return -1;
}

const KeywordMap& Parser::getKeyword(const int32_t& val)
{
    if (val != -1)
    {
        if (val >= 0 && val < KeywordTableSize)
            return KeywordTable[val];
    }
    return NullKeyword;
}

int32_t Parser::ActionIdx00(uint8_t ch)
{
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
                m_state = ST_INITIAL;

                // swap the string with the opcode
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
    m_state = ST_ID;
    if (!m_curString.empty())
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

enum Actions
{
    AC_IG = -3,  // Ignore this character
    AC_0L = -2,  // return TOK_EOL
    AC_0E = -1,  // error
    AC_00,       // push current character to token
    AC_01,       // if the token is keyword then return token TOK_MNEMONIC otherwise TOK_IDENTIFIER
    AC_02,       // add char to token then goto STATE ST_READ_ID
    AC_03,       // goto state ST_INITIAL, return TOK_LABEL
    AC_04,       // goto state ST_INITIAL, return TOK_IDENTIFIER
    AC_05,       // goto state ST_INITIAL and continue scanning
    AC_06,       // goto state ST_DIGIT and continue scanning
    AC_07,       // goto state ST_CONTINUE return TOK_DIGIT
    AC_SC,       // goto state ST_SECTION
    AC_DS,       // goto state ST_INITIAL and return TOK_SECTION
};

const Parser::StateTable Parser::States = {
    //[a-z] [A-Z]  [0-9]   ' '    '\n'    ','    '.'    ':'    '''    '"'    '('    ')'    '['    ']'    '#'   '+'     '-'    \0
    {AC_02, AC_02, AC_06, AC_01, AC_05, AC_0E, AC_SC, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_06, AC_01},  // ST_INITIAL
    {AC_00, AC_00, AC_00, AC_01, AC_01, AC_01, AC_IG, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_06, AC_01},  // ST_READ_ID
    {AC_0E, AC_0E, AC_00, AC_07, AC_07, AC_0E, AC_IG, AC_03, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_00, AC_01},  // ST_DIGIT
    {AC_0E, AC_0E, AC_0E, AC_0E, AC_0L, AC_0E, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_EXIT
    {AC_0E, AC_0E, AC_0E, AC_0E, AC_0L, AC_0E, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_ERROR
    {AC_0E, AC_0E, AC_0E, AC_05, AC_05, AC_05, AC_IG, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_CONTINUE
    {AC_00, AC_0E, AC_0E, AC_DS, AC_DS, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0E, AC_0L},  // ST_SECTION
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
            m_lineNo++;
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
        case '-':
            rc = CharType::CT_SUB;
            break;
        case '+':
            rc = CharType::CT_ADD;
            break;
        default:
            rc = CharType::CT_NULL;
            break;
        }
    }
    return rc;
}

const KeywordMap Parser::KeywordTable[] = {
    {"mov\0", OP_MOV, 2, AT_REG, AT_REGLIT, AT_NULL},
    {"ret\0", OP_RET, 0, AT_NULL, AT_NULL, AT_NULL},
    {"inc\0", OP_INC, 1, AT_REG, AT_NULL, AT_NULL},
    {"dec\0", OP_DEC, 1, AT_REG, AT_NULL, AT_NULL},
    {"cmp\0", OP_CMP, 2, AT_REGLIT, AT_REGLIT, AT_NULL},
    {"jmp\0", OP_JMP, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jeq\0", OP_JEQ, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jne\0", OP_JNE, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jlt\0", OP_JLT, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jgt\0", OP_JGT, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jle\0", OP_JLE, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"jge\0", OP_JGE, 1, AT_ADDR, AT_NULL, AT_NULL},
    {"prgi\0", OP_PRGI, 0, AT_NULL, AT_NULL, AT_NULL},
    {"prg\0", OP_PRG, 1, AT_REGLIT, AT_NULL, AT_NULL},
    {"add\0", OP_ADD, 2, AT_REG, AT_REGLIT, AT_NULL},
    {"sub\0", OP_SUB, 2, AT_REG, AT_REGLIT, AT_NULL},
    {"mul\0", OP_MUL, 2, AT_REG, AT_REGLIT, AT_NULL},
    {"div\0", OP_DIV, 2, AT_REG, AT_REGLIT, AT_NULL},
};

const size_t Parser::KeywordTableSize = sizeof(Parser::KeywordTable) / sizeof(KeywordMap);

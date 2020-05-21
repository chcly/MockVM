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
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "Declarations.h"

using namespace std;
const KeywordMap NullKeyword = {{'\0'}, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

inline bool isWhiteSpace(uint8_t ch);
inline bool isAlphaL(uint8_t ch);
inline bool isAlphaU(uint8_t ch);
inline bool isAlpha(uint8_t ch);
inline bool isDigit(uint8_t ch);
inline bool isNumber(uint8_t ch);
inline bool isEncodedNumber(uint8_t ch);
inline bool isAlphaNumeric(uint8_t ch);
inline bool isNewLine(uint8_t ch);
inline bool isTerminator(uint8_t ch);

Parser::Parser() :
    m_state(0),
    m_section(-1),
    m_label(-1),
    m_lineNo(1),
    m_labels(),
    m_instructions(),
    m_fname()
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
        int32_t sr, rc;
        m_section = -1;
        m_label   = 0;
        m_state   = ST_INITIAL;
        m_fname   = fname;

        while (!m_reader.eof())
        {
            Token curTok;
            sr = scan(curTok);
            switch (sr)
            {
            case TOK_OPCODE:
                rc = handleOpCode(curTok);
                break;
            case TOK_SECTION:
                rc = handleSection(curTok);
                break;
            case TOK_LABEL:
                rc = handleLabel(curTok);
                break;
            case PS_OK:
                return PS_OK;
            default:
                if (sr != PS_ERROR)
                    error("unhandled token %i\n", sr);
                rc = PS_ERROR;
                break;
            }
            if (rc != PS_OK)
            {
                error("syntax error\n");
                return PS_ERROR;
            }
        }
    }
    else
    {
        error("failed to load file %s.\n", fname);
        return PS_ERROR;
    }
    return PS_OK;
}

int32_t Parser::scan(Token& tok)
{
    int32_t res = PS_ERROR;
    while (!m_reader.eof())
    {
        switch (m_state)
        {
        case ST_INITIAL:
            res = handleInitialState(tok);
            break;
        case ST_ID:
            res = handleIdState(tok);
            break;
        case ST_DIGIT:
            res = handleDigitState(tok);
            break;
        case ST_SECTION:
            res = handleSectionState(tok);
            break;
        default:
            break;
        }

        if (res == PS_ERROR)
            return PS_ERROR;

        else if (res != ST_CONTINUE)
            return tok.type;
    }
    return PS_OK;
}

int32_t Parser::handleInitialState(Token& tok)
{
    uint8_t ch = m_reader.next();
    if (ch == '#')
    {
        scanEol();
        return ST_CONTINUE;
    }
    else if (ch == ';')
    {
        scanEol();
        return ST_CONTINUE;
    }
    else if (isNewLine(ch))
    {
        while (isNewLine(ch) && !m_reader.eof())
        {
            countNewLine(ch);
            ch = m_reader.next();
        }
        m_reader.offset(-1);
        return ST_CONTINUE;
    }

    if (isWhiteSpace(ch))
        ch = eatWhiteSpace(ch);

    if (isAlpha(ch))
    {
        m_reader.offset(-1);
        m_state = ST_ID;
    }
    else if (isDigit(ch) || ch == '-')
    {
        m_reader.offset(-1);
        m_state = ST_DIGIT;
    }
    else if (ch == '.')
    {
        m_state = ST_SECTION;
    }
    else if (ch == '#')
    {
        scanEol();
        return ST_CONTINUE;
    }
    else if (ch == ';')
    {
        scanEol();
        return ST_CONTINUE;
    }
    else if (!isNewLine(ch) && ch != 0)
    {
        error("undefined character '%c'\n", ch);
        return PS_ERROR;
    }
    return ST_CONTINUE;
}

int32_t Parser::handleIdState(Token& tok)
{
    uint8_t ch = m_reader.next();
    if (ch == '#')
    {
        scanEol();
        return ST_CONTINUE;
    }
    else if (ch == ';')
    {
        scanEol();
        return ST_CONTINUE;
    }

    if (isWhiteSpace(ch))
        ch = eatWhiteSpace(ch);

    if (isAlpha(ch))
    {
        while (isAlphaNumeric(ch) && !m_reader.eof())
        {
            tok.value.push_back(ch);
            ch = m_reader.next();
        }

        if (isTerminator(ch))
        {
            if (isWhiteSpace(ch))
            {
                ch = eatWhiteSpace(ch);
                m_reader.offset(-1);
                tok.hasComma = ch == ',';
            }
            else if (isNewLine(ch))
            {
                m_reader.offset(-1);
            }

            m_state = ST_INITIAL;
            if (tok.value.size() == 2 &&
                tok.value[1] >= '0' &&
                tok.value[1] <= '9')
            {
                if (tok.value[0] == 'x')
                {
                    tok.type = TOK_REGISTER;
                    tok.reg  = tok.value[1] - '0';
                    tok.value.clear();
                    return ST_MAX;
                }
            }

            size_t i = 0;
            for (i = 0; i < KeywordTableSize; ++i)
            {
                if (strncmp(KeywordTable[i].word, tok.value.c_str(), MAX_KWD) == 0)
                {
                    // swap the string with the opcode
                    tok.value.clear();
                    tok.op    = KeywordTable[i].op;
                    tok.type  = TOK_OPCODE;
                    tok.index = (int32_t)i;
                    return ST_MAX;
                }
            }

            tok.type = TOK_IDENTIFIER;
            return ST_MAX;
        }
        else if (ch == ':')
        {
            m_state  = ST_INITIAL;
            tok.type = TOK_LABEL;
            return ST_MAX;
        }
    }
    return ST_CONTINUE;
}

int32_t Parser::handleDigitState(Token& tok)
{
    uint8_t ch = m_reader.next();
    if (isWhiteSpace(ch))
        ch = eatWhiteSpace(ch);

    if (isNumber(ch) || ch == '-')
    {
        int  base    = 10;
        bool convert = ch == '0';
        bool b2 = false, b16 = false;

        while (isEncodedNumber(ch) && !m_reader.eof())
        {
            if (b2 && ch != '1' && ch != '0' && ch != 'b')
            {
                error("invalid binary number.\n");
                return PS_ERROR;
            }

            tok.value.push_back(ch);
            ch = m_reader.next();

            if (convert)
            {
                if (ch == 'x')
                    b16 = true;
                if (ch == 'b')
                    b2 = true;
            }
        }

        if (!b16 && !b2)
            convert = false;

        if (isTerminator(ch))
        {
            if (isWhiteSpace(ch))
            {
                ch = eatWhiteSpace(ch);
                m_reader.offset(-1);

                tok.hasComma = ch == ',';
            }
            else if (isNewLine(ch))
            {
                m_reader.offset(-1);
            }

            m_state  = ST_INITIAL;
            tok.type = TOK_DIGIT;

            if (convert)
            {
                tok.value = tok.value.substr(2, tok.value.size());
                if (b2)
                    base = 2;
                if (b16)
                    base = 16;
            }

            tok.ival.x = std::strtoull(tok.value.c_str(), nullptr, base);
            tok.value.clear();
            return ST_MAX;
        }
    }
    return ST_CONTINUE;
}

int32_t Parser::handleSectionState(Token& tok)
{
    uint8_t ch = m_reader.next();
    if (isWhiteSpace(ch))
        ch = eatWhiteSpace(ch);

    if (isAlphaL(ch))
    {
        while (isAlpha(ch) && !m_reader.eof())
        {
            tok.value.push_back(ch);
            ch = m_reader.next();
        }

        if (isNewLine(ch))
            m_reader.offset(-1);

        m_state  = ST_INITIAL;
        tok.type = TOK_SECTION;
        return ST_MAX;
    }

    error("undefined character. '%c'\n", ch);
    return PS_ERROR;
}

uint8_t Parser::scanEol(void)
{
    uint8_t ch = m_reader.current();
    while (!m_reader.eof() && (ch != '\n' && ch != '\r'))
        ch = m_reader.next();
    countNewLine(ch);
    return ch;
}

void Parser::countNewLine(uint8_t ch)
{
    // handle CRLF, LF, CR
    if (ch == '\r')
    {
        ch = m_reader.next();
        if (ch != '\n')
        {
            // CR, so put back the last char
            m_reader.offset(-1);
        }
        m_lineNo++;
    }
    else if (ch == '\n')
        m_lineNo++;
}

uint8_t Parser::eatWhiteSpace(uint8_t ch)
{
    while (isWhiteSpace(ch) && !m_reader.eof())
        ch = m_reader.next();
    return ch;
}

int32_t Parser::handleSection(const Token& tok)
{
    m_section = getSection(tok.value);
    if (m_section == UNDEFINED)
    {
        error("undefined section '%s'\n", tok.value.c_str());
        return PS_ERROR;
    }
    return PS_OK;
}

int32_t Parser::handleLabel(const Token& label)
{
    if (label.type == TOK_LABEL)
    {
        if (m_labels.find(label.value) != m_labels.end())
        {
            error("duplicate label '%s'\n", label.value.c_str());
            return PS_ERROR;
        }

        // This will be resolved after all text has been parsed.
        // For now this just needs to map to a unique index.
        m_labels[label.value] = ++m_label;
    }
    else
    {
        // should not happen unless Token::type is not the
        // same value that trigged this function.
        error("internal error, token type does not match the scanned token.\n");
        return PS_ERROR;
    }
    return PS_OK;
}

int32_t Parser::handleArgument(Instruction&      ins,
                               const KeywordMap& kwd,
                               const Token&      tok,
                               const int32_t     idx)
{
    if (kwd.argv[idx] == AT_REG)
    {
        if (tok.type != TOK_REGISTER)
        {
            error("expected operand one for %s, to be a register\n", kwd.word);
            return PS_ERROR;
        }

        ins.argv[idx] = tok.reg;
        ins.flags |= idx <= 0 ? (int)IF_DREG : (int)IF_SREG;
    }
    else if (kwd.argv[idx] == AT_LIT)
    {
        if (tok.type != TOK_DIGIT)
        {
            error("expected operand one for %s, to be a value\n", kwd.word);
            return PS_ERROR;
        }
        ins.argv[idx] = tok.ival.x;
        ins.flags |= idx <= 0 ? (int)IF_DLIT : (int)IF_SLIT;
    }
    else if (kwd.argv[idx] == AT_REGLIT)
    {
        if (tok.type == TOK_DIGIT)
        {
            ins.argv[idx] = tok.ival.x;
            ins.flags |= idx <= 0 ? (int)IF_DLIT : (int)IF_SLIT;
        }
        else if (tok.type == TOK_REGISTER)
        {
            ins.argv[idx] = tok.reg;
            ins.flags |= idx <= 0 ? (int)IF_DREG : (int)IF_SREG;
        }
        else
        {
            error("expected operand one for %s, to be a register or a value\n", kwd.word);
            return PS_ERROR;
        }
    }
    else if (kwd.argv[idx] == AT_ADDR)
    {
        // save this now so it can be resolved after all
        // labels have been stored
        ins.labelName = tok.value;
    }
    else
    {
        error("unknown first operand for %s\n", kwd.word);
        return PS_ERROR;
    }

    return PS_OK;
}

int32_t Parser::handleOpCode(const Token& tok)
{
    const KeywordMap& kwd = getKeyword(tok.index);
    if (kwd.op != UNDEFINED)
    {
        Instruction ins = {};

        ins.op    = kwd.op;
        ins.argc  = kwd.narg;
        ins.label = m_label;

        Token lastTok = {};
        if (ins.argc > 0)
        {
            if (scan(lastTok) == PS_ERROR)
                return PS_ERROR;

            if (handleArgument(ins, kwd, lastTok, 0) == PS_ERROR)
                return PS_ERROR;
        }

        // TODO, use lastTok.hasComma
        // to determine what should be scanned
        // next and also use it to add overloading
        // for example:
        //  add x0, x1     <- x0 = x0 + x1
        //  add x0, x1, x2 <- x0 = x1 + x2


        if (ins.argc > 1)
        {
            lastTok = {};
            if (scan(lastTok) == PS_ERROR)
                return PS_ERROR;

            if (handleArgument(ins, kwd, lastTok, 1) == PS_ERROR)
                return PS_ERROR;
        }

        m_instructions.push_back(ins);
        return PS_OK;
    }
    return PS_ERROR;
}

int32_t Parser::getSection(const std::string& val)
{
    if (val == "data")
        return SEC_DAT;
    else if (val == "text")
        return SEC_TXT;
    return UNDEFINED;
}

int32_t Parser::getKeywordIndex(const uint8_t& val)
{
    if (val == 0)
        return UNDEFINED;

    int32_t i;
    for (i = 0; i < KeywordTableSize; ++i)
    {
        if (KeywordTable[i].op == val)
            return i;
    }
    return UNDEFINED;
}

const KeywordMap& Parser::getKeyword(const int32_t& val)
{
    if (val != UNDEFINED)
    {
        if (val >= 0 && val < KeywordTableSize)
            return KeywordTable[val];
    }
    return NullKeyword;
}

const KeywordMap Parser::KeywordTable[] = {
    {"mov\0", OP_MOV, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"call\0", OP_CALL, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"ret\0", OP_RET, 0, {AT_NULL, AT_NULL, AT_NULL}},
    {"inc\0", OP_INC, 1, {AT_REG, AT_NULL, AT_NULL}},
    {"dec\0", OP_DEC, 1, {AT_REG, AT_NULL, AT_NULL}},
    {"cmp\0", OP_CMP, 2, {AT_REGLIT, AT_REGLIT, AT_NULL}},
    {"jmp\0", OP_JMP, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jeq\0", OP_JEQ, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jne\0", OP_JNE, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jlt\0", OP_JLT, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jgt\0", OP_JGT, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jle\0", OP_JLE, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"jge\0", OP_JGE, 1, {AT_ADDR, AT_NULL, AT_NULL}},
    {"prgi\0", OP_PRGI, 0, {AT_NULL, AT_NULL, AT_NULL}},
    {"prg\0", OP_PRG, 1, {AT_REGLIT, AT_NULL, AT_NULL}},
    {"add\0", OP_ADD, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"sub\0", OP_SUB, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"mul\0", OP_MUL, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"div\0", OP_DIV, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"shr\0", OP_SHR, 2, {AT_REG, AT_REGLIT, AT_NULL}},
    {"shl\0", OP_SHL, 2, {AT_REG, AT_REGLIT, AT_NULL}},
};

const size_t Parser::KeywordTableSize = sizeof(Parser::KeywordTable) / sizeof(KeywordMap);

inline bool isWhiteSpace(uint8_t ch)
{
    return ch == ' ' || ch == '\t';
}

inline bool isAlphaL(uint8_t ch)
{
    return ch >= 'a' && ch <= 'z';
}

inline bool isAlphaU(uint8_t ch)
{
    return ch >= 'A' && ch <= 'Z';
}

inline bool isAlpha(uint8_t ch)
{
    return isAlphaL(ch) || isAlphaU(ch) || ch == '_';
}

inline bool isDigit(uint8_t ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool isNumber(uint8_t ch)
{
    return ch >= '0' && ch <= '9';
}

inline bool isEncodedNumber(uint8_t ch)
{
    return isNumber(ch) || ch >= 'a' && ch <= 'f' || ch >= 'A' && ch <= 'F' || ch == 'x';
}

inline bool isAlphaNumeric(uint8_t ch)
{
    return isAlpha(ch) || isDigit(ch);
}

inline bool isNewLine(uint8_t ch)
{
    return ch == '\r' || ch == '\n';
}

inline bool isTerminator(uint8_t ch)
{
    return isWhiteSpace(ch) || ch == ',' || isNewLine(ch);
}

void Parser::error(const char* fmt, ...)
{
    if (fmt != nullptr)
    {
        va_list l1;
        int     s1, s2;

        char* buffer = nullptr;

        va_start(l1, fmt);
        s1 = std::vsnprintf(buffer, 0, fmt, l1);
        va_end(l1);

        if (s1 > 0)
        {
            buffer = (char*)malloc((size_t)s1 + 1);
            if (buffer)
            {
                va_start(l1, fmt);
                s2 = std::vsnprintf(buffer, (size_t)s1 + 1, fmt, l1);
                va_end(l1);
                
                fprintf(stdout, 
                    "%s(%i): error : %s", 
                    m_fname.c_str(), 
                    m_lineNo, buffer);
                
                free(buffer);
            }
        }
    }
}

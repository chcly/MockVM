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
#include <cstring>
#include <iostream>
#include "Declarations.h"

using namespace std;
const uint8_t    nullarg[3]  = {0xFF, 0xFF, 0xFF};
const KeywordMap NullKeyword = {{'\0'}, 0xFF, 0, nullarg};

Parser::Parser() :
    m_state(0),
    m_section(-1),
    m_label(-1),
    m_lineNo(1),
    m_labels(),
    m_instructions(),
    m_fname(),
    m_disableErrorFormat(false)
{
}

Parser::~Parser()
{
}


int32_t Parser::open(const char* fname)
{
    m_reader.open(fname);
    m_section = -1;
    m_label   = 0;
    m_state   = ST_INITIAL;
    m_fname   = fname;
    return m_reader.eof() ? (int32_t)PS_ERROR : (int32_t)PS_OK;
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
            case PS_EOF:
            case PS_OK:
            case ST_CONTINUE:
                return PS_OK;
            default:
                if (sr != PS_ERROR)
                {
                    str_t tok;
                    getTokenName(tok, sr);
                    error("token %s parsed outside of any known context\n", tok.c_str(), sr);
                }
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
    int32_t res = PS_EOF;

    // Always reset the token to a null token. 
    tok = {};

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
    return res;
}

int32_t Parser::handleInitialState(Token& tok)
{
    int32_t st = ST_CONTINUE;

    uint8_t ch = m_reader.next();
    if (ch == '#' || ch == ';')
        scanEol();
    else if (isNewLine(ch))
    {
        do
        {
            countNewLine(ch);
            ch = m_reader.next();
        } while (isNewLine(ch));

        if (ch != 0)
            m_reader.offset(-1);
    }
    else
    {
        if (isWhiteSpace(ch))
            ch = eatWhiteSpace(ch);

        if (ch != 0)
            m_reader.offset(-1);

        if (isAlpha(ch))
            m_state = ST_ID;
        else if (isDigit(ch) || ch == '-' || ch == '\'')
            m_state = ST_DIGIT;
        else if (ch == '.')
            m_state = ST_SECTION;
    }
    return st;
}

int32_t Parser::handleIdState(Token& tok)
{
    int32_t st = ST_CONTINUE;

    uint8_t ch = m_reader.next();
    if (ch == '#' || ch == ';')
        scanEol();
    else
    {
        if (isWhiteSpace(ch))
            ch = eatWhiteSpace(ch);

        if (isAlpha(ch))
        {
            do
            {
                tok.value.push_back(ch);
                ch = m_reader.next();
            } while (isAlphaNumeric(ch));

            if (isTerminator(ch))
                st = handleTermination(tok, ch);
            else if (ch == ':')
            {
                m_state  = ST_INITIAL;
                tok.type = TOK_LABEL;
                st = ST_MAX;
            }
            else
            {
                printf("%c\n", ch);
            }
        }
    }
    return st;
}


int32_t Parser::handleTermination(Token& tok, uint8_t ch)
{
    prepNextCall(tok, ch);

    m_state = ST_INITIAL;

    size_t sz = tok.value.size();
    if (sz == 2)
    {
        if (tok.value[1] >= '0' && tok.value[1] <= '9')
        {
            char ch = tok.value[0];
            if (ch == 'x' || ch == 'b' || ch == 'w' || ch == 'l')
            {
                switch (ch)
                {
                case 'b':
                    tok.regtype = IF_BTEB;
                    break;
                case 'w':
                    tok.regtype = IF_BTEW;
                    break;
                case 'l':
                    tok.regtype = IF_BTEL;
                    break;
                case 'x':
                default:
                    break;
                }
                tok.type = TOK_REGISTER;
                tok.reg  = tok.value[1] - '0';
                tok.value.clear();
                return ST_MAX;
            }
        }
        else if (tok.value[0] == 's' && tok.value[1] == 'p')
        {
            tok.type    = TOK_REGISTER;
            tok.reg     = 0;
            tok.regtype = IF_STKP;
            tok.value.clear();
            return ST_MAX;
        }
        else if (tok.value[0] == 'p' && tok.value[1] == 'c')
        {
            tok.type    = TOK_REGISTER;
            tok.reg     = 0;
            tok.regtype = IF_INSP;
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


int32_t Parser::handleDigitState(Token& tok)
{
    int32_t st = ST_CONTINUE;

    uint8_t ch = m_reader.next();
    if (isWhiteSpace(ch))
        ch = eatWhiteSpace(ch);

    if (isNumber(ch) || ch == '-')
    {
        int  base    = 10;
        bool convert = ch == '0';
        bool b2 = false, b16 = false;

        while (isEncodedNumber(ch) && st != PS_ERROR)
        {
            if (b2 && ch != '1' && ch != '0' && ch != 'b')
            {
                error("invalid binary number\n");
                st = PS_ERROR;
            }
            else
            {
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
        }

        if (!b16 && !b2)
            convert = false;

        if (isTerminator(ch) && st != PS_ERROR)
        {
            prepNextCall(tok, ch);

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
            st = ST_MAX;
        }
        else
        {
            if (st != PS_ERROR)
            {
                error("invalid termination\n");
                st = PS_ERROR;
            }
        }
    }
    else if (ch == '\'')
        st = handleCharacter(tok, ch);
    return st;
}

int32_t Parser::handleCharacter(Token& tok, uint8_t ch)
{
    char v, n;
    ch = m_reader.next();
    v  = ch;

    n = m_reader.next();
    if (ch == '\\')
    {
        switch (n)
        {
        case 'n':
            v = '\n';
            break;
        case 't':
            v = '\t';
            break;
        case 'r':
            v = '\r';
            break;
        case '\'':
            v = '\'';
            break;
        case '\\':
            v = '\\';
            break;
        default:
            error("escape sequence '%c' not handled\n", n);
            return PS_ERROR;
        }
        n = m_reader.next();
    }

    if (n != '\'')
    {
        error("invalid character\n");
        return PS_ERROR;
    }

    ch = m_reader.next();
    prepNextCall(tok, ch);

    m_state    = ST_INITIAL;
    tok.type   = TOK_DIGIT;
    tok.ival.x = (int)v;
    tok.value.clear();
    return ST_MAX;
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
    while (ch != '\n' && ch != '\r' && ch != 0)
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
        if (ch != '\n' && ch != 0)
        {
            // CR, so put back the last char
            m_reader.offset(-1);
        }
        m_lineNo++;
    }
    else if (ch == '\n')
        m_lineNo++;
}

void Parser::prepNextCall(Token& tok, uint8_t ch)
{
    // trim any white space and
    // if there is a comma, flag the token
    // indicating there is another argument
    // that comes next
    if (isWhiteSpace(ch))
    {
        ch = eatWhiteSpace(ch);
        m_reader.offset(-1);
        tok.hasComma = ch == ',';
    }
    else if (ch == ',')
        tok.hasComma = true;
}

uint8_t Parser::eatWhiteSpace(uint8_t ch)
{
    while (isWhiteSpace(ch))
        ch = m_reader.next();
    return ch;
}

int32_t Parser::handleSection(const Token& tok)
{
    m_section = getSection(tok.value);
    if (m_section == PS_UNDEFINED)
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

void Parser::markInstructionAsRegister(Instruction& ins, const Token& tok, int idx)
{
    ins.argv[idx] = tok.reg;
    switch (idx)
    {
    case 0:
        ins.flags |= IF_REG0;
        break;
    case 1:
        ins.flags |= IF_REG1;
        break;
    case 2:
        ins.flags |= IF_REG2;
        break;
    }

    if (tok.regtype == IF_STKP)
        ins.flags |= IF_STKP;
    else if (tok.regtype == IF_INSP)
        ins.flags |= IF_INSP;
}

int32_t Parser::handleArgument(Instruction&      ins,
                               const KeywordMap& kwd,
                               const Token&      tok,
                               const int32_t     idx)
{
    if (kwd.argv[idx] == AT_REGI)
    {
        if (tok.type != TOK_REGISTER)
        {
            errorArgType(idx, tok.type, kwd.word);
            return PS_ERROR;
        }

        markInstructionAsRegister(ins, tok, idx);

    }
    else if (kwd.argv[idx] == AT_SVAL)
    {
        if (tok.type != TOK_DIGIT)
        {
            errorArgType(idx, tok.type, kwd.word);
            return PS_ERROR;
        }
        ins.argv[idx] = tok.ival.x;
    }
    else if (kwd.argv[idx] == AT_RVAL)
    {
        if (tok.type == TOK_DIGIT)
        {
            ins.argv[idx] = tok.ival.x;
        }
        else if (tok.type == TOK_REGISTER)
        {
            markInstructionAsRegister(ins, tok, idx);
        }
        else
        {
            errorArgType(idx, tok.type, kwd.word);
            return PS_ERROR;
        }
    }
    else if (kwd.argv[idx] == AT_ADDR)
    {
        // save this now so it can be resolved after all
        // labels have been stored
        ins.flags |= IF_ADDR;
        ins.lname = tok.value;
    }
    else
    {
        error("unknown operand type for %s\n", kwd.word);
        errorTokenType(tok.type);
        return PS_ERROR;
    }

    return PS_OK;
}

int32_t Parser::handleOpCode(const Token& tok)
{
    const KeywordMap& kwd = getKeyword(tok.index);

    if (kwd.op != PS_UNDEFINED)
    {
        Instruction ins = {};
        ins.op          = kwd.op;
        ins.argc        = kwd.narg;
        ins.label       = m_label;

        Token lastTok = {};
        int   arg     = 0;
        int   maxArg  = ins.argc;

        for (arg = 0; arg < maxArg; ++arg)
        {
            if (arg > 0 && !lastTok.hasComma)
            {
                error("Missing comma after operand %i.\n", arg + 1);
                error("%s expects %i arguments.\n", kwd.word, ins.argc);
                return PS_ERROR;
            }
            if (scan(lastTok) == PS_ERROR)
                return PS_ERROR;

            if (handleArgument(ins, kwd, lastTok, arg) == PS_ERROR)
                return PS_ERROR;

            if (kwd.argv[INS_ARG - 1] != AT_NULL)
            {
                if (lastTok.hasComma && (arg + 1) == ins.argc)
                {
                    if (arg + 2 == INS_ARG)
                    {
                        maxArg += 1;
                        ins.argc += 1;
                    }
                }
            }
        }

        if (lastTok.hasComma)
        {
            error("too many arguments supplied to %s\n", kwd.word);
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
    return PS_UNDEFINED;
}

int32_t Parser::getKeywordIndex(const uint8_t& val)
{
    if (val == 0)
        return PS_UNDEFINED;

    int32_t i;
    for (i = 0; i < KeywordTableSize; ++i)
    {
        if (KeywordTable[i].op == val)
            return i;
    }
    return PS_UNDEFINED;
}

const KeywordMap& Parser::getKeyword(const int32_t& val)
{
    if (val != PS_UNDEFINED)
    {
        if (val >= 0 && val < KeywordTableSize)
            return KeywordTable[val];
    }
    return NullKeyword;
}

bool Parser::isWhiteSpace(uint8_t ch)
{
    return ch == ' ' || ch == '\t';
}

bool Parser::isAlphaL(uint8_t ch)
{
    return ch >= 'a' && ch <= 'z';
}

bool Parser::isAlphaU(uint8_t ch)
{
    return ch >= 'A' && ch <= 'Z';
}

bool Parser::isAlpha(uint8_t ch)
{
    return isAlphaL(ch) || isAlphaU(ch) || ch == '_';
}

bool Parser::isDigit(uint8_t ch)
{
    return ch >= '0' && ch <= '9';
}

bool Parser::isNumber(uint8_t ch)
{
    return ch >= '0' && ch <= '9' || ch == '-';
}

bool Parser::isEncodedNumber(uint8_t ch)
{
    return isNumber(ch) || ch >= 'a' && ch <= 'f' || ch >= 'A' && ch <= 'F' || ch == 'x';
}

bool Parser::isAlphaNumeric(uint8_t ch)
{
    return isAlpha(ch) || isDigit(ch);
}

bool Parser::isNewLine(uint8_t ch)
{
    return ch == '\r' || ch == '\n';
}

bool Parser::isTerminator(uint8_t ch)
{
    return isWhiteSpace(ch) || ch == ',' || isNewLine(ch) || ch == 0;
}

void Parser::errorTokenType(int tok)
{
    str_t str;
    getTokenName(str, tok);
    error("found type '%s' instead\n", str.c_str());
}

void Parser::errorArgType(int idx, int tok, const char* inst)
{
    error("expected operand %i for %s, to be a register\n", idx + 1, inst);
    errorTokenType(tok);
}

void Parser::getTokenName(str_t& name, int tok)
{
    switch (tok)
    {
    case TOK_OPCODE:
        name = "instruction";
        break;
    case TOK_REGISTER:
        name = "register";
        break;
    case TOK_IDENTIFIER:
        name = "identifier";
        break;
    case TOK_DIGIT:
        name = "value";
        break;
    case TOK_LABEL:
        name = "label";
        break;
    case TOK_SECTION:
        name = "section";
        break;
    default:
        name = "undefined";
        break;
    }
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

                if (!m_disableErrorFormat)
                {
                    fprintf(stdout,
                            "%s(%i): error : %s",
                            m_fname.c_str(),
                            m_lineNo,
                            buffer);
                }
                else
                {
                    fprintf(stdout,
                            "(%i): error : %s",
                            m_lineNo,
                            buffer);
                }

                free(buffer);
            }
        }
    }
}

const uint8_t ArgTypeStd1[3] = {AT_REGI, AT_RVAL, AT_NULL};
const uint8_t ArgTypeStd2[3] = {AT_RVAL, AT_RVAL, AT_NULL};
const uint8_t ArgTypeStd3[3] = {AT_RVAL, AT_NULL, AT_NULL};
const uint8_t ArgTypeStd4[3] = {AT_REGI, AT_RVAL, AT_RVAL};
const uint8_t ArgTypeReg1[3] = {AT_REGI, AT_NULL, AT_NULL};
const uint8_t ArgTypeAdr1[3] = {AT_ADDR, AT_NULL, AT_NULL};
const uint8_t ArgTypeNone[3] = {AT_REGI, AT_RVAL, AT_NULL};

const KeywordMap Parser::KeywordTable[] = {
    {"mov\0 ", OP_MOV, 2, ArgTypeStd1},
    {"call\0", OP_GTO, 1, ArgTypeAdr1},
    {"ret\0 ", OP_RET, 0, ArgTypeNone},
    {"inc\0 ", OP_INC, 1, ArgTypeReg1},
    {"dec\0 ", OP_DEC, 1, ArgTypeReg1},
    {"cmp\0 ", OP_CMP, 2, ArgTypeStd2},
    {"jmp\0 ", OP_JMP, 1, ArgTypeAdr1},
    {"jeq\0 ", OP_JEQ, 1, ArgTypeAdr1},
    {"jne\0 ", OP_JNE, 1, ArgTypeAdr1},
    {"jlt\0 ", OP_JLT, 1, ArgTypeAdr1},
    {"jgt\0 ", OP_JGT, 1, ArgTypeAdr1},
    {"jle\0 ", OP_JLE, 1, ArgTypeAdr1},
    {"jge\0 ", OP_JGE, 1, ArgTypeAdr1},
    {"prgi\0", OP_PRI, 0, ArgTypeAdr1},
    {"prg\0 ", OP_PRG, 1, ArgTypeStd3},
    {"add\0 ", OP_ADD, 2, ArgTypeStd4},
    {"sub\0 ", OP_SUB, 2, ArgTypeStd4},
    {"mul\0 ", OP_MUL, 2, ArgTypeStd4},
    {"div\0 ", OP_DIV, 2, ArgTypeStd4},
    {"shr\0 ", OP_SHR, 2, ArgTypeStd4},
    {"shl\0 ", OP_SHL, 2, ArgTypeStd4},
};

const size_t Parser::KeywordTableSize = sizeof(Parser::KeywordTable) / sizeof(KeywordMap);

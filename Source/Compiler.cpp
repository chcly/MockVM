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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#include "StateTable.inl"



struct Token
{
    uint8_t op;
    uint8_t destination;
    uint8_t source;
    size_t  position;
    size_t  namelen;
    uint64_t ival;
    string   value;


    Token() :
        op(0x00),
        destination(0x00),
        source(0x00),
        position(0),
        namelen(0),
        ival(0),
        value()
    {
    }
};


typedef int32_t (*Action)(int32_t &st, uint8_t ch, Token &dest);



struct Instruction
{
    uint8_t  op, namelen;
    uint32_t dst;
    uint32_t src;
    uint32_t location;
    string   value;

    Instruction() :
        op(0), 
        namelen(0), 
        dst(0),
        src(0),
        location(0),
        value()
    {
    }
};


struct Header
{
    char   magic[3];
    size_t data;
    size_t string;
    size_t text;
};

class BlockReader
{
public:
    static const size_t BLOCKSIZE = 1024;
    typedef uint8_t     Block[BLOCKSIZE + 1];

private:
    Block  m_block;
    size_t m_fileLen;
    size_t m_loc;
    FILE * m_fp;

    void read(void);
    void open(const char *fname);

public:
    BlockReader(const char *fname);
    ~BlockReader();
    bool    eof(void);
    uint8_t next(void);
};

int32_t getCharType(uint8_t ct);
void    usage(void);
int32_t lex(BlockReader &blr, Token &tok, int32_t &st);

int32_t ActionIdx00(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx01(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx02(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx03(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx04(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx05(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx06(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdx07(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdxSC(int32_t &st, uint8_t ct, Token &dest);
int32_t ActionIdxDC(int32_t &st, uint8_t ct, Token &dest);


const Action ActionTable[] = {
    ActionIdx00,
    ActionIdx01,
    ActionIdx02,
    ActionIdx03,
    ActionIdx04,
    ActionIdx05,
    ActionIdx06,
    ActionIdx07,
    ActionIdxSC,
    ActionIdxDC,
    nullptr,
};
const size_t ActionTableSize = sizeof(ActionTable) / sizeof(ActionTable[0]);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    vector<string> files;

    int    i;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            files.push_back(argv[i]);
    }

    for (string file : files)
    {
        BlockReader blr(file.c_str());
        int st = ParserState::ST_INITIAL, rv, rc;
        Token       tok;

        while (!blr.eof())
        {
            rc = lex(blr, tok, st);
            if (rc == -1)
                break;

            if (rc == TOK_SECTION)
            {
                // handle
                cout << "     .";
                cout << tok.value << '\n';
            }
            else if (rc == TOK_LABEL)
                cout << tok.value << ':' << '\n';
            else if (rc == TOK_MNEMONIC)
            {
                cout << "  ";
                cout << hex << (int)tok.op << ' ';
                rv= lex(blr, tok, st);
                if (rv == TOK_REGISTER)
                {
                    cout << 'x' << (int)tok.source << ' ';
                    rv = lex(blr, tok, st);
                    if (rv == TOK_REGISTER)
                        cout << hex << 'x' << (int)tok.source << ' ';
                    else if (rv == TOK_DIGIT)
                        cout << hex << (int)tok.ival << ' ';
                
                }
                cout << '\n';
            }
        }
    }
    return 0;
}


int32_t lex(BlockReader &blr, Token &tok, int32_t &st)
{
    while (!blr.eof())
    {
        uint8_t ct   = blr.next();
        int32_t type = getCharType(ct);
        int32_t act   = States[st][type];

        if (act >= 0 && act < ActionTableSize)
        {
            if (ActionTable[act] != nullptr)
            {
                int rc = ActionTable[act](st, ct, tok);
                if (rc != -1)
                    return rc;
            }
        }
        else if (act == AC_0E)
            cout << "Unknown character parsed '" << ct << "'\n";
    }
    return -1;
}



void usage(void)
{
}

int getCharType(uint8_t ct)
{
    CharType rc = CharType::CT_NULL;
    if (ct >= 'a' && ct <= 'z')
        rc = CharType::CT_LALPHA;
    else if (ct >= 'A' && ct <= 'Z')
        rc= CharType::CT_UALPHA;
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




BlockReader::BlockReader(const char *fname) :
    m_block(),
    m_fileLen(0),
    m_loc(0),
    m_fp(0)
{
    open(fname);
}

BlockReader::~BlockReader()
{
    if (m_fp)
        fclose(m_fp);
}

bool BlockReader::eof(void)
{
    return m_loc > m_fileLen;
}

uint8_t BlockReader::next(void)
{
    if (m_loc <= 0 || (m_loc % BLOCKSIZE) == 0)
        read();

    uint8_t rc = 0;

    if (m_loc < m_fileLen)
        rc = m_block[m_loc % BLOCKSIZE];
    m_loc++;
    return rc;
}


void BlockReader::read()
{
    size_t br   = fread(m_block, 1, BLOCKSIZE, m_fp);
    if (br >= 0 && br <= BLOCKSIZE)
        m_block[br] = 0;
}

void BlockReader::open(const char *fname)
{
    if (fname)
    {
        m_fp = fopen(fname, "rb");
        if (m_fp)
        {
            fseek(m_fp, 0L, SEEK_END);
            m_fileLen = ftell(m_fp);
            fseek(m_fp, 0L, SEEK_SET);
        }
        else
            puts("failed to open file.");
    }
    else
        puts("Invalid file name.");
}



int32_t ActionIdx00(int32_t &st, uint8_t ct, Token &dest)
{
    dest.value.push_back(ct);
    return -1;
}


int32_t ActionIdx01(int32_t &st, uint8_t ct, Token &dest)
{
    if (!dest.value.empty())
    {
        st = ST_CONTINUE;

        if (dest.value.size() == 2 && dest.value[1] >= '0' && dest.value[1] <= '9')
        {
            const char *cp = dest.value.c_str();
            if (cp[0] == 'x')
            {
                dest.source = cp[1] -  '0';
                return TOK_REGISTER;
            }
        }

        size_t i = 0;
        for (i = 0; i < MnemonicTableSize; ++i)
        {
            if (strncmp(MnemonicTable[i].word, dest.value.c_str(), 6) == 0)
            {
                dest.value.clear();
                dest.op = MnemonicTable[i].op;
                return TOK_MNEMONIC;
            }
        }
        return TOK_IDENTIFIER;
    }
    return -1;
}


int32_t ActionIdx02(int32_t &st, uint8_t ct, Token &dest)
{
    dest.value.push_back(ct);
    st = ST_READ_ID;
    return -1;
}

int32_t ActionIdx03(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_INITIAL, return TOK_LABEL
    st = ST_INITIAL;
    return TOK_LABEL;
}

int32_t ActionIdx04(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_INITIAL, return TOK_IDENTIFIER
    st = ST_INITIAL;
    return TOK_IDENTIFIER;
}

int32_t ActionIdx05(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_INITIAL, continues
    st = ST_INITIAL;
    dest.value.clear();
    return -1;
}

int32_t ActionIdx06(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_DIGIT
    st = ST_DIGIT;
    dest.value.clear();
    dest.value.push_back(ct);
    return -1;
}

int32_t ActionIdx07(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_CONTINUE
    st = ST_CONTINUE;
    dest.ival = atoi(dest.value.c_str());
    return TOK_DIGIT;
}
int32_t ActionIdxSC(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_SECTION
    st = ST_SECTION;
    dest.value.clear();
    return -1;
}

int32_t ActionIdxDC(int32_t &st, uint8_t ct, Token &dest)
{
    // goto STATE ST_INITIAL return TOK_SECTION
    st = ST_INITIAL;
    return TOK_SECTION;
}


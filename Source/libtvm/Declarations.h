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
#ifndef _Declarations_h_
#define _Declarations_h_

#include <stdint.h>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include "ArrayStack.h"
#include "SharedLib.h"

#define INS_ARG 3
#define MAX_KWD 5
#define MAX_REG 10

// Maximum number of branches present at one time
#define MAX_STK 256

typedef std::string        str_t;
typedef std::vector<str_t> strvec_t;
typedef std::set<str_t>    strset_t;

typedef union Register
{
    uint8_t  b[8];
    uint16_t w[4];
    uint32_t l[2];
    uint64_t x;
} Register;

typedef Register Registers[MAX_REG];

enum RegisterArg
{
    A0_1 = 0x001,
    A0_2 = 0x002,
    A0_4 = 0x004,
    A1_1 = 0x010,
    A1_2 = 0x020,
    A1_4 = 0x040,
    A2_1 = 0x100,
    A2_2 = 0x200,
    A2_4 = 0x400,  // uint16_t
};

const uint16_t SizeFlags[3][3] = {
    {A0_1, A0_2, A0_4},
    {A1_1, A1_2, A1_4},
    {A2_1, A2_2, A2_4},
};

enum ProgramFlags
{
    PF_Z = 1 << 0,
    PF_G = 1 << 1,
    PF_L = 1 << 2,
};

struct Token
{
    uint8_t  op;
    uint8_t  reg;
    uint16_t regtype;
    uint16_t sectype;
    Register ival;
    int32_t  type;
    str_t    value;
    int32_t  index;
    bool     hasComma;
};

struct DataDeclaration
{
    str_t    lname;
    uint16_t type;
    str_t    sval;
    uint64_t ival;
};

enum ParseResult
{
    PS_ERROR = -4,
    PS_EOF,
    // Common error return code
    PS_UNDEFINED,
    // return value for actions that need to scan
    // for more information before returning a token.
    PS_CONTINUE,
    PS_OK,  // Keep at zero
    PS_MAX
};

enum ParserState
{
    ST_INITIAL = PS_MAX + 1,
    ST_ID,
    ST_DIGIT,
    ST_ASCII,
    ST_SECTION,
    ST_INDEX,
    ST_CONTINUE,
    ST_MAX,
};

enum TokenCode
{
    TOK_OPCODE = ST_MAX + 1,
    TOK_REGISTER,
    TOK_REGINDEX,
    TOK_IDENTIFIER,
    TOK_DIGIT,
    TOK_ASCII,
    TOK_LABEL,
    TOK_SECTION,
    TOK_MAX,
};

enum Opcode
{
    // The value here needs to start at zero
    // and match the method table index in the program.
    // class. The only reason is to prevent having to do
    // a lookup during execution. If OP_BEG ever needs to
    // change, the offset can also be found by subtracting
    // off the difference then using that as the index.
    OP_BEG = 0,  // unused padding
    OP_RET,      // ret
    OP_MOV,      // mov r(n), src
    OP_GTO,      // call address
    OP_INC,      // inc, r(n)
    OP_DEC,      // dec, r(n)
    OP_CMP,      // cmp, r(n), src
    OP_JMP,      // jump
    OP_JEQ,      // jump ==
    OP_JNE,      // jump !
    OP_JLT,      // jump <
    OP_JGT,      // jump >
    OP_JLE,      // jump <=
    OP_JGE,      // jump >=
    OP_ADD,      // add r(n), src
    OP_SUB,      // sub r(n), src
    OP_MUL,      // mul r(n), src
    OP_DIV,      // div r(n), src
    OP_SHR,      // shr r(n), src
    OP_SHL,      // shl r(n), src
    OP_ADRP,     // adrp r(n), addr
    OP_STR,      // str r(n),  [r(n), index]
    OP_LDR,      // ldr  r(n), [r(n), index]
    OP_LDRS,     // ldsr r(n), [r(n), index]
    OP_STRS,     // strs r(n), [r(n), index]
    OP_STP,      // stp r(n), val
    OP_LDP,      // ldp r(n), val
    // ---- debugging ----
    OP_PRG,  // print register
    OP_PRI,  // print all registers
             // ---- debugging ----
    OP_MAX,  // uint8_t
};

enum ArgType
{
    AT_NULL,
    AT_REGI,
    AT_SVAL,
    AT_ADDR,
    AT_RVAL,
    AT_RVAA,
    AT_RIDX,
};

typedef char Keyword[MAX_KWD + 1];

struct KeywordMap
{
    Keyword        word;
    uint8_t        op;
    uint8_t        narg;
    const uint8_t* argv;
};

enum SectionCodes
{
    // File sections
    SEC_DAT = 0xFF,
    SEC_TXT,
    // Data sections
    SEC_DECL_ST,
    SEC_ASCII,  // .asciz
    SEC_BYTE,   // .byte
    SEC_WORD,   // .word
    SEC_LONG,   // .long
    SEC_QUAD,   // .quad | .xword
    SEC_ZERO,   // Reserve a block of zeroed memory
    SEC_DECL_EN,
};

enum InstructionFlags
{
    IF_REG0 = 0x0001,
    IF_REG1 = 0x0002,
    IF_REG2 = 0x0004,
    IF_ADDR = 0x0008,
    IF_ADRD = 0x0010,
    IF_SYMU = 0x0020,
    IF_STKP = 0x0040,  // stack pointer
    IF_INSP = 0x0080,  // instruction pointer
    IF_BTEB = 0x0100,  // b uint8_t
    IF_BTEW = 0x0200,  // w uint16_t
    IF_BTEL = 0x0400,  // l uint32_t
                       // x = default, if not present
    IF_RIDX = 0x0800,  // [r(n), idx < 256]
    IF_MAXF = 0x1000,  // needs an uint16_t
};

struct TVMHeader
{
    uint8_t  code[2];
    uint16_t flags;
    uint32_t dat;
    uint32_t str;
    uint32_t sym;
};

struct TVMSection
{
    uint16_t flags;
    uint16_t align;
    uint32_t entry;
    uint32_t size;
    uint32_t start;
};

struct Instruction
{
    uint8_t  op;
    uint8_t  argc;
    uint16_t flags;
    uint16_t sizes;
    uint8_t  index;
    uint64_t argv[INS_ARG];
    uint64_t label;
    uint64_t sym;
    str_t    lname;
};

typedef void (*Symbol)(tvmregister_t);

struct SymbolTable
{
    const char* name;
    Symbol      callback;
};

typedef SymbolTable* (*ModuleInit)();

struct ExecInstruction
{
    uint8_t  op;
    uint8_t  argc;
    uint16_t flags;
    uint64_t argv[INS_ARG];
    uint16_t index;
    Symbol   call;
};

using Instructions     = std::vector<Instruction>;
using IndexToPosition  = std::unordered_map<uint64_t, uint64_t>;
using LabelMap         = std::unordered_map<str_t, uint64_t>;
using SymbolMap        = std::unordered_map<str_t, Symbol>;
using StringLookup     = std::unordered_map<str_t, str_t>;
using AddressLookup    = std::unordered_map<str_t, uint64_t>;
using DynamicLib       = std::vector<void*>;
using StringMap        = std::unordered_map<str_t, uint64_t>;
using ExecInstructions = std::vector<ExecInstruction>;
using DataLookup       = std::unordered_map<str_t, DataDeclaration>;

#define _TIME_CHECK_BEGIN                                             \
    {                                                                 \
        chrono::high_resolution_clock::time_point begintick, endtick; \
        begintick = chrono::high_resolution_clock().now();            \
        {
#define _TIME_CHECK_END                                               \
    }                                                                 \
    endtick = chrono::high_resolution_clock().now();                  \
    cout << __FUNCTION__ << " exec("                                  \
         << fixed << setprecision(6)                                  \
         << ((chrono::duration<double>(endtick - begintick).count())) \
         << "s)"                                                      \
         << endl;                                                     \
    }

#endif  // _Declarations_h_

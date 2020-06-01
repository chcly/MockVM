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
#ifndef _Program_h_
#define _Program_h_

#include <stdint.h>
#include <stack>
#include <unordered_map>
#include <vector>
#include "BlockReader.h"
#include "Declarations.h"
#include "MemoryStream.h"

class Program
{
public:
    typedef void (Program::*Operation)(const ExecInstruction& inst);
    typedef Operation InstructionTable[OP_MAX - OP_BEG];

private:
    ExecInstructions m_ins;
    TVMHeader        m_header;
    Registers        m_regi;
    uint32_t         m_flags;
    int32_t          m_return;
    uint64_t         m_curinst;
    LabelMap         m_strtab;
    strvec_t         m_strtablist;
    ArrayStack       m_callStack;
    str_t            m_modpath;
    DynamicLib       m_dynlib;
    SymbolMap        m_symbols;
    MemoryStream     m_dataTable;
    ArrayStack       m_stack;

    const static InstructionTable OPCodeTable;
    const static size_t           OPCodeTableSize;

    int findDynamic(ExecInstruction& ins);

    void handle_OP_RET(const ExecInstruction& inst);
    void handle_OP_MOV(const ExecInstruction& inst);
    void handle_OP_CALL(const ExecInstruction& inst);
    void handle_OP_INC(const ExecInstruction& inst);
    void handle_OP_DEC(const ExecInstruction& inst);
    void handle_OP_CMP(const ExecInstruction& inst);
    void handle_OP_JMP(const ExecInstruction& inst);
    void handle_OP_JEQ(const ExecInstruction& inst);
    void handle_OP_JNE(const ExecInstruction& inst);
    void handle_OP_JLE(const ExecInstruction& inst);
    void handle_OP_JGE(const ExecInstruction& inst);
    void handle_OP_JLT(const ExecInstruction& inst);
    void handle_OP_JGT(const ExecInstruction& inst);
    void handle_OP_ADD(const ExecInstruction& inst);
    void handle_OP_SUB(const ExecInstruction& inst);
    void handle_OP_MUL(const ExecInstruction& inst);
    void handle_OP_DIV(const ExecInstruction& inst);
    void handle_OP_SHR(const ExecInstruction& inst);
    void handle_OP_SHL(const ExecInstruction& inst);
    void handle_OP_ADRP(const ExecInstruction& inst);
    void handle_OP_STP(const ExecInstruction& inst);
    void handle_OP_LDP(const ExecInstruction& inst);
    void handle_OP_STR(const ExecInstruction& inst);
    void handle_OP_LDR(const ExecInstruction& inst);
    void handle_OP_LDRS(const ExecInstruction& inst);
    void handle_OP_STRS(const ExecInstruction& inst);
    void handle_OP_PRG(const ExecInstruction& inst);
    void handle_OP_PRGI(const ExecInstruction& inst);

    void forceExit(int returnCode);

    int  loadStringTable(BlockReader& reader);
    int  loadSymbolTable(BlockReader& reader);
    int  loadDataTable(BlockReader& reader);
    int  loadCode(BlockReader& reader);
    bool testInstruction(const ExecInstruction& exec);

    Register* clone(void);
    void      release(Register*);

public:
    Program(const str_t& modpath);
    ~Program();

    int load(const char* fname);
    int launch(void);
};

#endif  //_Program_h_

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
#ifndef _BinaryWriter_h_
#define _BinaryWriter_h_

#include "Declarations.h"

class BinaryWriter
{
private:
    void*           m_fp;
    long            m_loc;
    Instructions    m_ins;
    size_t          m_sizeOfCode;
    size_t          m_sizeOfData;
    size_t          m_sizeOfSym;
    size_t          m_sizeOfStr;
    IndexToPosition m_addrMap;
    LabelMap        m_labels;
    LabelMap        m_strtab;
    strvec_t        m_orderedString;
    strset_t        m_linkedLibraries;
    SymbolLookup    m_symbols;
    TVMHeader       m_header;
    str_t           m_modpath;


    void   write(const void* v, size_t size);
    void   write8(uint8_t v);
    void   write16(uint16_t v);
    void   write32(uint32_t v);
    void   write64(uint64_t v);

    size_t writeDataSection(void);
    size_t writeCodeSection(void);
    size_t writeSymbolSection(void);
    size_t writeStringSection(void);
    int    mapInstructions(void);
    size_t calculateInstructionSize(void);

    size_t findLabel(const str_t& name);
    size_t addToStringTable(const str_t& symname);
    size_t addLinkedSymbol(const str_t& symname, const str_t& libname);
    int    loadSharedLibrary(const str_t& lib);

public:
    BinaryWriter(const str_t& modpath);
    ~BinaryWriter();

    void mergeInstructions(const Instructions& insl);

    int mergeLabels(const LabelMap& map);
    int resolve(strvec_t& modules);
    int open(const char* fname);
    int writeHeader(void);
    int writeSections(void);
};

#endif  //_BinaryWriter_h_

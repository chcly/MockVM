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

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include "Instruction.h"

class BinaryWriter
{
public:
    typedef std::vector<Instruction>                Instructions;
    typedef std::unordered_map<int64_t, int64_t>    IndexToPosition;
    typedef std::unordered_map<std::string, size_t> LabelMap; 

private:
    void*            m_fp;
    long             m_loc;
    Instructions     m_ins;
    IndexToPosition  m_addrMap;
    LabelMap         m_labels;

    void write(void* v, size_t size);
    void write8(uint8_t v);
    void write16(uint16_t v);
    void write32(uint32_t v);
    void write64(uint64_t v);

    size_t computeInstructionSize(const Instruction& ins);
    size_t mapInstructions(void);


    int64_t findLabel(const std::string& name); 


public:

    BinaryWriter();
    ~BinaryWriter();

    void mergeInstructions(const Instructions& insl);
    void mergeLabels(const LabelMap& map);

    void open(const char* fname);
    void writeHeader();
    void writeSections();
};

#endif  //_BinaryWriter_h_

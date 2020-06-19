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
#ifndef _InstructionWriter_h_
#define _InstructionWriter_h_

#include "Types.h"
#include <sstream>

class InstructionWriter
{
private:
    std::ostringstream    m_os;
    const ExecInstruction m_inst;
public:
    InstructionWriter(const ExecInstruction &inst);

    void  writeOp(void);
    void  writeSpace(void);
    void  writePC(void);
    void  writeSP(void);
    void  writeRegister(int index);
    void  writeValue(int index, int width=-1);
    void  writeIndex(void);
    void  writeRegIndex(void);
    void  writeCall(void);
    void  writeAddrD(size_t v);
    void  writeNext(void);
    void  openBrace(void);
    void  closeBrace(void);
    str_t string(void);
};


#endif  //_InstructionWriter_h_

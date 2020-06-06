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
#ifndef _Debugger_h_
#define _Debugger_h_

#include "Program.h"
class Console;

class Debugger : public Program
{
private:
    str_t    m_file;
    Console* m_console;
    bool     m_exit;
    int16_t  m_ypos;

    Registers m_last;

private:
    void displayHeader(void);
    void displayInstructions(void);
    void displayRegisters(void);
    void displayStack(void);

    void render(void);
    void step(void);

    void disassemble(const ExecInstruction& inst, size_t i);

    void getOpString(str_t& dest, const uint8_t op);

public:
    Debugger(const str_t& mod, const str_t& file);
    ~Debugger();

    int debug(void);
};

#endif  //_Debugger_h_

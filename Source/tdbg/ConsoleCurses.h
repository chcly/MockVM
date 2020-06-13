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
#ifndef _ConsoleCurses_h_
#define _ConsoleCurses_h_

#include <stdio.h>
#include "Console.h"

class ConsoleCurses : public Console
{
private:
    uint8_t *m_buffer;
    uint8_t *m_colorBuffer;
    FILE *   m_stdout;
    
    uint8_t getSwappedColor(uint8_t inp);
    void    writeChar(char ch, uint8_t col, size_t k);

    void readRedirectedOutput(const str_t &_path);

public:
    ConsoleCurses();
    virtual ~ConsoleCurses();

    int  nextCommand();
    void clear();
    void flush();
    int  create();
    void switchOutput(bool on);
    void setCursorPosition(int x, int y);
    void showCursor(bool doit);
};

#endif  //_ConsoleCurses_h_

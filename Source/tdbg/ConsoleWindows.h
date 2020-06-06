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
#ifndef _ConsoleWindows_h_
#define _ConsoleWindows_h_

#include <windows.h>
#include "Console.h"

class ConsoleWindows : public Console
{
private:
    CHAR_INFO *m_buffer;
    CHAR_INFO *m_startBuf;
    SMALL_RECT m_startRect;
    str_t      m_tmpFile;
    COORD      m_startCurs;
  
    size_t     m_size;
    HANDLE     m_stdout;
    FILE *     m_redirect;
    
    uint32_t getColorImpl(ColorSpace fg,
                          ColorSpace bg);
    void     writeChar(char ch, uint32_t col, size_t k);

    void makeTempFileName();


public:
    ConsoleWindows();
    virtual ~ConsoleWindows();

    size_t getNextCmd();
    void   clear();
    void   flush();
    int    create();

    void switchOutput(bool on);

    void setCursorPosition(int x, int y);
    void showCursor(bool doit);
};

#endif  //_ConsoleWindows_h_

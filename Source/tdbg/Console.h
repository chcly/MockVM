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
#ifndef _Console_h_
#define _Console_h_

#include "Color.h"
#include "Declarations.h"

enum ConsoleCtrlStatus
{
    CCS_NO_INPUT=0,
    CCS_QUIT,
    CCS_STEP
};

class Console
{
protected:
    str_t    m_std;
    int16_t  m_width;
    int16_t  m_height;
    uint32_t m_curColor;

    virtual uint32_t getColorImpl(ColorSpace fg,
                                  ColorSpace bg) = 0;

    virtual void writeChar(char ch, uint32_t col, size_t k) = 0;

public:
    Console();
    virtual ~Console();

    void displayString(const str_t &string, int16_t x, int16_t y);
    void displayChar(char ch, int16_t x, int16_t y);
    void displayOutput(int16_t x, int16_t y);
    void clearOutput();
    
    void displayLineHorz(int16_t st, int16_t en, int16_t y);
    void displayLineVert(int16_t st, int16_t en, int16_t x);

    void setColor(ColorSpace fg, ColorSpace bg = ColorSpace::CS_TRANSPARENT);


    virtual int  getNextCmd()                    = 0;
    virtual void clear()                         = 0;
    virtual void flush()                         = 0;
    virtual int  create()                        = 0;
    virtual void switchOutput(bool on)           = 0;
    virtual void setCursorPosition(int x, int y) = 0;
    virtual void showCursor(bool doit)           = 0;


    inline size_t getWidth()
    {
        return m_width;
    }

    inline size_t getHeight()
    {
        return m_height;
    }
};

extern Console *GetPlatformConsole();

#endif  //_Console_h_

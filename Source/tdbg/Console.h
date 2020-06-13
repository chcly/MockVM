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

#include "Types.h"

class Console
{
protected:
    str_t       m_std;
    size_t      m_size;
    ConsoleRect m_displayRect;
    uint8_t     m_curColor;
    int16_t     m_lineCount;
    ConsoleRect m_maxOutput;

    virtual uint8_t getColorImpl(uint8_t fg, uint8_t bg)      = 0;
    virtual void    writeChar(char ch, uint8_t col, size_t k) = 0;

public:
    Console();
    virtual ~Console();

    void displayLineHorz(int16_t st, int16_t en, int16_t y);
    void displayLineVert(int16_t st, int16_t en, int16_t x);
    void displayString(const str_t &string, int16_t x, int16_t y);
    void displayChar(char ch, int16_t x, int16_t y);
    void displayCharHex(int ch, int16_t x, int16_t y);
    void displayInt(int v, int16_t x, int16_t y);
    void displayIntRightAligned(int v, int16_t r, int16_t y);

    void    displayOutput(int16_t x, int16_t y);
    int16_t getOutputLineCount();
    void    clearOutput();
    void    appendOutput(const str_t &str);
    void    setColor(uint8_t fg, uint8_t bg = CS_TRANSPARENT);

    int          pause();
    virtual int  getNextCmd()                    = 0;
    virtual void clear()                         = 0;
    virtual void flush()                         = 0;
    virtual int  create()                        = 0;
    virtual void switchOutput(bool on)           = 0;
    virtual void setCursorPosition(int x, int y) = 0;
    virtual void showCursor(bool doit)           = 0;

    inline size_t getWidth()
    {
        return (size_t)m_displayRect.w;
    }

    inline size_t getHeight()
    {
        return (size_t)m_displayRect.h;
    }

    const ConsoleRect &getRect()
    {
        return m_displayRect;
    }

    inline void setOutputRect(const ConsoleRect &pt)
    {
        m_maxOutput = pt;
    }

    inline ConsoleRect &getOutputRect()
    {
        return m_maxOutput;
    }
};

extern Console *GetPlatformConsole();

#endif  //_Console_h_

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

class Console
{
protected:
    uint8_t *m_buffer;
    uint8_t *m_colorBuffer;

    int16_t  m_width;
    int16_t  m_height;
    uint32_t m_curColor;

    virtual uint32_t getColorImpl(ColorSpace fg,
                                  ColorSpace bg = ColorSpace::CS_TRANSPARENT) = 0;

public:
    Console();
    virtual ~Console();

    void displayString(const str_t &string, size_t x, size_t y);
    void displayChar(char ch, size_t x, size_t y);

    void displayLineHorz(size_t st, size_t en, size_t y);
    void displayLineVert(size_t st, size_t en, size_t x);

    void setColor(ColorSpace fg, ColorSpace bg = ColorSpace::CS_TRANSPARENT);

    virtual size_t getNextCmd() = 0;

    virtual size_t getWidth()  = 0;
    virtual size_t getHeight() = 0;

    virtual void clear()  = 0;
    virtual void flush()  = 0;
    virtual int  create() = 0;

    virtual void setCursorPosition(int x, int y) = 0;
    virtual void showCursor(bool doit)           = 0;
};

extern Console *GetPlatformConsole();

#endif  //_Console_h_

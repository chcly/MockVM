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
#include "Console.h"
#ifdef _WIN32
#include "ConsoleWindows.h"
#else
#include "ConsoleCurses.h"
#endif  // _WIN32

Console::Console() :
    m_width(0),
    m_height(0),
    m_curColor(CS_WHITE)
{
}

Console::~Console()
{
}

void Console::displayString(const str_t &string, int16_t x, int16_t y)
{
    int16_t i, s, k, b;

    if (y < 0)
        return;
    if (x < 0 || x > m_width)
        return;

    s = (int16_t)string.size();

    const char *ptr = string.c_str();

    b = (size_t)m_width * (size_t)m_height;
    for (i = 0; i < s; ++i)
    {
        k = x + i;
        if (k < 0 || y < 0)
            continue;
        if (k > m_width)
            continue;
        if (y > m_height)
            continue;

        k += y * m_width;

        if (k <= b)
            writeChar(ptr[i], m_curColor, k);
    }
}

void Console::displayChar(char ch, int16_t x, int16_t y)
{
    if (y < 0 || y > m_height)
        return;
    if (x < 0 || x > m_width)
        return;

    int16_t k = x + y * m_width;
    if (k < m_width * m_height)
        writeChar(ch, m_curColor, k);
}

void Console::displayLineHorz(int16_t st, int16_t en, int16_t y)
{
    int16_t i;
    for (i = st; i < en; ++i)
        displayChar('-', i, y);
}

void Console::displayLineVert(int16_t st, int16_t en, int16_t x)
{
    int16_t i;
    for (i = st; i < en; ++i)
        displayChar('|', x, i);
}

void Console::displayOutput(int16_t x, int16_t y)
{
    int16_t st  = x;
    size_t  len = m_std.size(), i;
    for (i = 0; i < len; i++)
    {
        char ch = m_std.at(i);
        if (ch == '\n' || x + 1 > m_width)
        {
            x = st;
            ++y;
        }
        else
            displayChar(ch, x++, y);
    }
}

void Console::clearOutput()
{
    m_std.clear();
}


void Console::readRedirectedOutput(const str_t &_path)
{
    FILE *fp = fopen(_path.c_str(), "r");
    if (fp)
    {
        fseek(fp, 0L, SEEK_END);
        long len = ftell(fp);
        if (len > 0)
        {
            fseek(fp, 0L, SEEK_SET);
            char buffer[256] = {};

            len = fread(buffer, 1, 255, fp);
            if (len > 0)
                m_std += str_t(buffer, len);
        }

        fclose(fp);
    }
}


void Console::setColor(ColorSpace fg, ColorSpace bg)
{
    m_curColor = getColorImpl(fg, bg);
}

Console *GetPlatformConsole()
{
    Console *ret;

#ifdef _WIN32
    ret = new ConsoleWindows();
#else
    ret = new ConsoleCurses();
#endif  // _WIN32
    if (ret->create() != 0)
    {
        printf("failed to create platform console.\n");
        delete ret;
        ret = nullptr;
    }

    return ret;
}

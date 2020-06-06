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
#include "ConsoleCurses.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include "SymbolUtils.h"

struct RGB
{
    int r, g, b;
};

#define Imin 0
#define Imax 1000
#define Imid 700
#define Icen 500

ConsoleCurses::ConsoleCurses() :
    m_supportsColor(false)
{
    memset(m_colorTable, 0, 256);
}

ConsoleCurses ::~ConsoleCurses()
{
    endwin();
}

size_t ConsoleCurses::getWidth()
{
    return m_width;
}

size_t ConsoleCurses::getHeight()
{
    return m_height;
}

void ConsoleCurses::clear()
{
    if (m_buffer)
    {
        memset(m_buffer, ' ', m_width * m_height);
        memset(m_colorBuffer, 0, m_width * m_height);
    }
}

size_t ConsoleCurses::getNextCmd()
{
    int ch = getch();
    if (ch == 'q')
        return 27;

    if (ch == KEY_DOWN)
        return 13;

    return 0;
}

void ConsoleCurses::setCursorPosition(int x, int y)
{
    move(x, y);
}

void ConsoleCurses::showCursor(bool doit)
{
    curs_set(doit ? 1 : 0);
}

void ConsoleCurses::flush()
{
    move(0, 0);
    curs_set(0);
    int c = 0;

    size_t i, j, k;
    for (i = 1; i < m_height; ++i)
    {
        for (j = 1; j < m_width; ++j)
        {
            k = j + (i * m_width);

            if (m_colorBuffer[k] != c)
            {
                c = m_colorBuffer[k];
                attron(COLOR_PAIR(c));
            }

            move(i, j);
            delch();
            insch(m_buffer[k]);
        }
    }
    move(80, 0);
    refresh();
}

uint32_t ConsoleCurses::getColorImpl(ColorSpace fg, ColorSpace bg)
{
    if (!m_supportsColor)
        return 0;

    return m_colorTable[bg % 16][fg % 16];
}

void ConsoleCurses::mapEnumColor(int mapping, int fg, int bg)
{
    int fgcol = COLOR_WHITE;
    int bgcol = COLOR_BLACK;

    RGB fgi = {};

    switch (fg)
    {
    case CS_GREY:
        fgi = {Icen, Icen, Icen};
        break;
    case CS_LIGHT_GREY:
        fgi = {Imid, Imid, Imid};
        break;
    case CS_WHITE:
        fgi = {Imax, Imax, Imax};
        break;
    case CS_BLACK:
        fgi   = {72, 72, 72};
        fgcol = COLOR_BLACK;
        break;
    case CS_DARKBLUE:
        fgi   = {Imin, Imin, Imid};
        fgcol = COLOR_BLUE;
        break;
    case CS_BLUE:
        fgi   = {Imin, Imin, Imax};
        fgcol = COLOR_BLUE;
        break;
    case CS_DARKGREEN:
        fgi   = {Imin, Imid, Imin};
        fgcol = COLOR_GREEN;
        break;
    case CS_GREEN:
        fgi   = {Imin, Imax, Imin};
        fgcol = COLOR_GREEN;
        break;
    case CS_DARKCYAN:
        fgi   = {Imin, Imid, Imid};
        fgcol = COLOR_CYAN;
        break;
    case CS_CYAN:
        fgi   = {Imin, Imax, Imax};
        fgcol = COLOR_CYAN;
        break;
    case CS_DARKRED:
        fgi   = {Imid, Imin, Imin};
        fgcol = COLOR_RED;
        break;
    case CS_RED:
        fgi   = {Imax, Imin, Imin};
        fgcol = COLOR_RED;
        break;
    case CS_DARKMAGENTA:
        fgi   = {Imid, Imin, Imid};
        fgcol = COLOR_MAGENTA;
        break;
    case CS_MAGENTA:
        fgi   = {Imax, Imin, Imax};
        fgcol = COLOR_MAGENTA;
        break;
    case CS_DARKYELLOW:
        fgi   = {Imid, Imid, Imin};
        fgcol = COLOR_YELLOW;
        break;
    case CS_YELLOW:
        fgi   = {Imax, Imax, Imin};
        fgcol = COLOR_YELLOW;
        break;
    }

    switch (bg)
    {
    default:
    case CS_BLACK:
        break;
    case CS_GREY:
    case CS_LIGHT_GREY:
    case CS_WHITE:
        bgcol = COLOR_WHITE;
        break;
    case CS_DARKBLUE:
    case CS_BLUE:
        bgcol = COLOR_BLUE;
        break;
    case CS_DARKGREEN:
    case CS_GREEN:
        bgcol = COLOR_GREEN;
        break;
    case CS_DARKCYAN:
    case CS_CYAN:
        bgcol = COLOR_CYAN;
        break;
    case CS_DARKRED:
    case CS_RED:
        bgcol = COLOR_RED;
        break;
    case CS_DARKMAGENTA:
    case CS_MAGENTA:
        bgcol = COLOR_MAGENTA;
        break;
    case CS_DARKYELLOW:
    case CS_YELLOW:
        bgcol = COLOR_YELLOW;
        break;
    }

    if (can_change_color())
        init_color(fgcol, fgi.r, fgi.g, fgi.b);

    init_pair(mapping, fgcol, bgcol);
}

int ConsoleCurses::create()
{
    initscr();

    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    start_color();
    m_supportsColor = has_colors() != 0;

    if (m_supportsColor)
    {
        int i, j, c;
        for (i = 0; i < 16; ++i)
        {
            for (j = 0; j < 16; ++j)
            {
                c = i + j * 16;
                mapEnumColor(c, j, i);
                m_colorTable[i][j] = c;
            }
        }
    }

    int r, c;
    getmaxyx(stdscr, r, c);

    m_width  = c;
    m_height = r;

    size_t size = r * c;

    m_buffer      = new uint8_t[size];
    m_colorBuffer = new uint8_t[size];

    memset(m_buffer, ' ', size);
    memset(m_colorBuffer, 0, size);

    return 0;
}

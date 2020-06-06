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
#include <fcntl.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

ConsoleCurses::ConsoleCurses() :
    m_stdout(nullptr),
    m_supportsColor(false),
    m_buffer(nullptr),
    m_colorBuffer(nullptr),
    m_size(0)
{
    memset(m_colorTable, 0, 256);
}

ConsoleCurses ::~ConsoleCurses()
{
    delete []m_buffer;
    delete[] m_colorBuffer;

    endwin();
}

void ConsoleCurses::clear()
{
    if (m_buffer)
    {
        memset(m_buffer, ' ', m_width * m_height);
        memset(m_colorBuffer, 0, m_width * m_height);
    }
}

void ConsoleCurses::switchOutput(bool on)
{
    if (on)
        m_stdout = freopen("/tmp/tdbg_stdout", "w", stdout);
    else
    {
        if (m_stdout != nullptr)
            fclose(m_stdout);
        freopen("/dev/tty", "w", stdout);
    }

    readRedirectedOutput("/tmp/tdbg_stdout");
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
    for (i = 0; i < m_height; ++i)
    {
        for (j = 0; j < m_width; ++j)
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
    refresh();
}

uint32_t ConsoleCurses::getColorImpl(ColorSpace fg, ColorSpace bg)
{
    if (!m_supportsColor)
        return 0;

    if (bg == CS_TRANSPARENT)
        bg = CS_BLACK;

    return m_colorTable[bg][fg % 16];
}

void ConsoleCurses::writeChar(char ch, uint32_t col, size_t k)
{
    if (k < m_size)
    {
        m_buffer[k]      = ch;
        m_colorBuffer[k] = (uint8_t)col;
    }
}


void ConsoleCurses::mapEnumColor(int mapping, int fg, int bg)
{
    int fgcol = COLOR_WHITE;
    int bgcol = COLOR_BLACK;
    switch (fg)
    {
    case CS_GREY:
        break;
    case CS_LIGHT_GREY:
        break;
    case CS_WHITE:
        break;
    case CS_BLACK:
        fgcol = COLOR_BLACK;
        break;
    case CS_DARKBLUE:
        fgcol = COLOR_BLUE;
        break;
    case CS_BLUE:
        fgcol = COLOR_BLUE;
        break;
    case CS_DARKGREEN:
        fgcol = COLOR_GREEN;
        break;
    case CS_GREEN:
        fgcol = COLOR_GREEN;
        break;
    case CS_DARKCYAN:
        fgcol = COLOR_CYAN;
        break;
    case CS_CYAN:
        fgcol = COLOR_CYAN;
        break;
    case CS_DARKRED:
        fgcol = COLOR_RED;
        break;
    case CS_RED:
        fgcol = COLOR_RED;
        break;
    case CS_DARKMAGENTA:
        fgcol = COLOR_MAGENTA;
        break;
    case CS_MAGENTA:
        fgcol = COLOR_MAGENTA;
        break;
    case CS_DARKYELLOW:
        fgcol = COLOR_YELLOW;
        break;
    case CS_YELLOW:
        fgcol = COLOR_YELLOW;
        break;
    }

    if (bg != 0)
    {
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
            bgcol = COLOR_MAGENTA;
            break;
        case CS_MAGENTA:
            bgcol = COLOR_MAGENTA;
            break;
        case CS_DARKYELLOW:
        case CS_YELLOW:
            bgcol = COLOR_YELLOW;
            break;
        }
    }
    else
        bgcol = -1;

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

    use_default_colors();

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

    m_size  = r * c;
    m_buffer      = new uint8_t[m_size];
    m_colorBuffer = new uint8_t[m_size];

    memset(m_buffer, ' ', m_size);
    memset(m_colorBuffer, 0, m_size);

    return 0;
}

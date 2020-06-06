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
#include "ConsoleWindows.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

void ClearConsole(void *stdhandle);

ConsoleWindows::ConsoleWindows() :
    m_stdout(nullptr),
    m_stdin(nullptr),
    m_lastSize({0, 0}),
    m_lastParm(0)
{
}

ConsoleWindows::~ConsoleWindows()
{
    if (m_stdout)
        ClearConsole(m_stdout);
}

size_t ConsoleWindows::getWidth()
{
    return m_width;
}

size_t ConsoleWindows::getHeight()
{
    return m_height;
}

void ConsoleWindows::clear()
{
    size_t size = (size_t)m_width * (size_t)m_height;
    memset(m_buffer, ' ', size);
    memset(m_colorBuffer, CS_WHITE, size);
}

size_t ConsoleWindows::getNextCmd()
{
    if (GetAsyncKeyState(VK_ESCAPE))
        return 27;

    if (GetAsyncKeyState(VK_DOWN))
    {
        Sleep(100);
        return 13;
    }
    Sleep(100);
    return 0;
}

void ConsoleWindows::setCursorPosition(int x, int y)
{
    SetConsoleCursorPosition(m_stdout, {(short)x, (short)y});
}

void ConsoleWindows::showCursor(bool doit)
{
    CONSOLE_CURSOR_INFO cinf;
    cinf.bVisible = doit ? 1 : 0;
    cinf.dwSize   = 1;
    SetConsoleCursorInfo(m_stdout, &cinf);
}

void ConsoleWindows::flush()
{
    ConsoleWindows::showCursor(false);
    ConsoleWindows::setCursorPosition(0, 0);

    uint32_t   color = ConsoleWindows::getColorImpl(ColorSpace::CS_WHITE, ColorSpace::CS_TRANSPARENT);
    const char nl    = '\n';

    int16_t i, j, k;
    for (i = 0; i < m_height; ++i)
    {
        for (j = 0; j < m_width; ++j)
        {
            k = j + (i * m_width);

            if (m_colorBuffer[k] != color)
            {
                color = m_colorBuffer[k];
                ::SetConsoleTextAttribute(m_stdout, color);
            }

            ::WriteConsole(
                m_stdout,
                &m_buffer[k],
                1,
                nullptr,
                nullptr);
        }
        ::WriteConsole(
            m_stdout,
            &nl,
            1,
            nullptr,
            nullptr);
    }

    ConsoleWindows::setCursorPosition(0, 0);
}

int ConsoleWindows::create()
{
    m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_stdout == INVALID_HANDLE_VALUE)
    {
        printf("failed to acquire stdout\n");
        return -1;
    }

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(m_stdout, &info) == 0)
        return -1;

    CONSOLE_CURSOR_INFO cinf;
    cinf.bVisible = 0;
    cinf.dwSize   = 1;
    SetConsoleCursorInfo(m_stdout, &cinf);

    if (m_width == 0)
        m_width = info.srWindow.Right;
    if (m_height == 0)
        m_height = info.srWindow.Bottom;
    if (m_height > 30)
        m_height = 30;

    size_t size   = (size_t)m_width * (size_t)m_height;
    m_buffer      = new uint8_t[size + 1];
    m_colorBuffer = new uint8_t[size + 1];
    memset(m_colorBuffer, CS_WHITE, size);


    return 0;
}

const unsigned char COLOR_TABLE[16][16] = {
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F},
    {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F},
    {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F},
    {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F},
    {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F},
    {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F},
    {0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F},
    {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF},
    {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF},
    {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF},
    {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF},
    {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF},
    {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF},
};

uint32_t ConsoleWindows::getColorImpl(ColorSpace fg, ColorSpace bg)
{
    int ivf = (int)fg;

    if (bg != ColorSpace::CS_TRANSPARENT)
    {
        int ivb = (int)bg;
        if (ivf < 16)
            return COLOR_TABLE[ivb][ivf];
        else
            return (int)ColorSpace::CS_WHITE;
    }
    else
    {
        if (ivf < 16)
            return COLOR_TABLE[(int)ColorSpace::CS_BLACK][ivf];
        else
            return (int)ColorSpace::CS_WHITE;
    }
}

void ClearConsole(void *stdhandle)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(stdhandle, &info) == 0)
        return;
    SetConsoleCursorPosition(stdhandle, {0, 0});

    const char spc   = ' ';
    const char nl    = '\n';
    uint8_t    color = COLOR_TABLE[CS_BLACK][CS_WHITE];

    DWORD tmp;

    int16_t i, j;
    for (i = 0; i < info.srWindow.Bottom; ++i)
    {
        for (j = 0; j < info.srWindow.Right; ++j)
        {
            FillConsoleOutputCharacter(stdhandle,
                                       spc,
                                       1,
                                       {j, i},
                                       &tmp);

            FillConsoleOutputAttribute(stdhandle,
                                       color,
                                       1,
                                       {j, i},
                                       &tmp);
        }
    }

    SetConsoleCursorPosition(stdhandle, {0, 0});
}

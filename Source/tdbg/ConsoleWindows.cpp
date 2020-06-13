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
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "MemoryStream.h"
#include "SymbolUtils.h"

#define STDOUT 1
#define VK_Q 0x51
#define VK_R 0x52
#define VK_C 0x43
#define VK_Z 0x5A

const CHAR_INFO NullChar = {' ', CS_WHITE};

void        ZeroBufferMemory(CHAR_INFO *dest, size_t size);
BOOL WINAPI CtrlCallback(DWORD evt);

ConsoleWindows::ConsoleWindows() :
    m_buffer(nullptr),
    m_startBuf(nullptr),
    m_startRect({0, 0, 0, 0}),
    m_stdout(nullptr),
    m_redirIn(nullptr),
    m_redirOut(nullptr),
    m_dup(0),
    m_redir(nullptr),
    m_fd(0),
    m_oldMode(0)
{
    initialize();
}

ConsoleWindows::~ConsoleWindows()
{
    finalize();
    delete[] m_buffer;
    delete[] m_startBuf;
}

void ConsoleWindows::initialize()
{
    // in, out, err, redir
    _setmaxstdio(4);

    SECURITY_ATTRIBUTES attr;
    attr.bInheritHandle       = TRUE;
    attr.lpSecurityDescriptor = nullptr;

    if (::CreatePipe(&m_redirIn, &m_redirOut, &attr, 1024) == FALSE)
        printf("failed to create pipe\n");

    if (!m_dup)
        m_dup = _dup(STDOUT);
}

void ConsoleWindows::finalize()
{
    if (m_stdout)
    {
        SetConsoleCursorPosition(m_stdout, m_startCurs);
        CONSOLE_CURSOR_INFO cinf;
        cinf.bVisible = 1;
        cinf.dwSize   = 1;
        SetConsoleCursorInfo(m_stdout, &cinf);

        // Restore the contents of
        // the start screen.
        WriteConsoleOutput(
            m_stdout,
            m_startBuf,
            {m_displayRect.w, m_displayRect.h},
            {0, 0},
            &m_startRect);

        SetConsoleMode(m_stdin, m_oldMode);
    }

    CloseHandle(m_redirIn);

    if (m_redir)
    {
        // closes, m_fd, m_redirOut
        fclose(m_redir);
    }
}

void ConsoleWindows::clear()
{
    ZeroBufferMemory(m_buffer, m_size);
}

void ConsoleWindows::switchOutput(bool on)
{
    if (on)
    {
        if (m_redirOut)
        {
            if (!m_fd)
            {
                // transfers ownership of m_redirOut,
                // (do not close m_redirOut with CloseHandle, nor m_fd with close)
                m_fd = _open_osfhandle((intptr_t)m_redirOut, _O_TEXT);
            }

            if (m_fd != -1)
            {
                // associate m_redirOut with stdout.
                if (!m_redir)
                    m_redir = _fdopen(m_fd, "w");

                if (_dup2(_fileno(m_redir), STDOUT) == -1)
                    printf("Failed to duplicate stdout file descriptor\n");
            }
            else
            {
                printf("failed to open low level file descriptor\n");
            }
        }
    }
    else
    {
        DWORD br;
        br = GetFileSize(m_redirIn, nullptr);
        if (br > 0 && br != -1)
        {
            MemoryStream ms;
            ms.reserve(br);

            ReadFile(m_redirIn, ms.ptr(), br, &br, nullptr);
            if (br > 0)
                m_std += str_t((char *)ms.ptr(), br);
        }

        // re associate stdout with the main console.
        if (m_dup)
            _dup2(m_dup, STDOUT);
    }
}

int ConsoleWindows::processKeyEvent(const KEY_EVENT_RECORD &rec)
{
    int rc = CCS_NO_INPUT;

    if (rec.bKeyDown)
    {
        if (rec.wVirtualKeyCode == VK_F11)
            rc = CCS_STEP;
        else if (rec.wVirtualKeyCode == VK_F10)
            rc = CCS_STEP;
        else if (rec.wVirtualKeyCode == VK_F9)
            rc = CCS_ADD_BREAKPOINT;
        else if (rec.wVirtualKeyCode == VK_F5)
            rc = CCS_CONTINUE;
        else if (rec.wVirtualKeyCode == VK_Q)
            rc = CCS_QUIT;
        else if (rec.wVirtualKeyCode == VK_R)
            rc = CCS_RESTART;
        else if (rec.dwControlKeyState & LEFT_CTRL_PRESSED ||
                 rec.dwControlKeyState & RIGHT_CTRL_PRESSED)
        {
            if (rec.wVirtualKeyCode == VK_C ||
                rec.wVirtualKeyCode == VK_Z)
            {
                rc = CCS_FORCE_EXIT;
            }
        }
    }
    return rc;
}

int ConsoleWindows::processSizeEvent(const WINDOW_BUFFER_SIZE_RECORD &rec)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(m_stdout, &info) == FALSE)
    {
        printf("failed get the screen buffer\n");
        return CCS_NO_INPUT;
    }

    DWORD oldSize   = (DWORD)m_size,
          oldRight  = m_startRect.Right,
          oldBottom = m_startRect.Bottom;

    size_t i, j, k;

    m_displayRect = {0, 0, info.srWindow.Right, info.srWindow.Bottom};
    m_size        = (size_t)m_displayRect.w * (size_t)m_displayRect.h;
    m_startRect   = {0, 0, info.srWindow.Right, info.srWindow.Bottom};

    CHAR_INFO *newBuf = new CHAR_INFO[m_size];
    ZeroBufferMemory(newBuf, m_size);

    for (j = 0; j < oldBottom; ++j)
    {
        for (i = 0; i < oldRight; ++i)
        {
            k = j + i * oldRight;
            if (k < oldSize && k < m_size)
                newBuf[k] = m_startBuf[k];
        }
    }

    delete[] m_startBuf;
    m_startBuf = newBuf;

    delete[] m_buffer;
    m_buffer = new CHAR_INFO[m_size];
    ZeroBufferMemory(m_buffer, m_size);

    return CCS_RESIZE;
}

int ConsoleWindows::getNextCmd()
{
    INPUT_RECORD ir[16];
    DWORD        nr, i;
    int          rc = CCS_NO_INPUT;

block_input:

    WaitForSingleObject(m_stdin, INFINITE);

    if (ReadConsoleInput(m_stdin, ir, 16, &nr))
    {
        for (i = 0; i < nr && rc == CCS_NO_INPUT; ++i)
        {
            switch (ir[i].EventType)
            {
            case WINDOW_BUFFER_SIZE_EVENT:
                rc = processSizeEvent(ir[i].Event.WindowBufferSizeEvent);
                break;
            case KEY_EVENT:
                rc = processKeyEvent(ir[i].Event.KeyEvent);
                break;
            default:
            case MENU_EVENT:
            case FOCUS_EVENT:
            case MOUSE_EVENT:
                break;
            }
        }
    }

    if (rc == CCS_NO_INPUT)
    {
        // continue to filter for wanted events.
        goto block_input;
    }
    else
    {
        if (FlushConsoleInputBuffer(m_stdin) == FALSE)
        {
            printf("failed to flush the input buffer\n");
        }
    }
    return rc;
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

void ConsoleWindows::writeChar(char ch, uint8_t col, size_t k)
{
    if (k < m_size)
    {
        m_buffer[k].Char.AsciiChar = ch;
        m_buffer[k].Attributes     = (WORD)col;
    }
}

void ConsoleWindows::flush()
{
    if (!m_stdout)
        return;

    ConsoleWindows::showCursor(false);
    ConsoleWindows::setCursorPosition(m_displayRect.x, m_displayRect.y);

    SMALL_RECT sr = {m_displayRect.x, m_displayRect.y, m_displayRect.w, m_displayRect.h};
    WriteConsoleOutput(
        m_stdout,
        m_buffer,
        {m_displayRect.w, m_displayRect.h},
        {m_displayRect.x, m_displayRect.y},
        &sr);
}

int ConsoleWindows::create()
{
    m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (m_stdout == INVALID_HANDLE_VALUE)
    {
        printf("failed to acquire stdout\n");
        return -1;
    }

    m_stdin = GetStdHandle(STD_INPUT_HANDLE);
    if (m_stdin == INVALID_HANDLE_VALUE)
    {
        printf("failed to acquire stdin\n");
        return -1;
    }

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(m_stdout, &info) == FALSE)
    {
        printf("failed get the screen buffer\n");
        return -1;
    }

    SetConsoleCtrlHandler(CtrlCallback, TRUE);

    GetConsoleMode(m_stdin, &m_oldMode);

    DWORD newMode = 0;
    newMode |= ENABLE_WINDOW_INPUT;
    newMode |= ENABLE_MOUSE_INPUT;
    SetConsoleMode(m_stdin, newMode);

    m_displayRect = {0, 0, info.srWindow.Right, info.srWindow.Bottom};
    m_size        = (size_t)m_displayRect.w * (size_t)m_displayRect.h;
    m_buffer      = new CHAR_INFO[m_size];

    ZeroBufferMemory(m_buffer, m_size);

    ConsoleWindows::showCursor(false);

    m_startBuf  = new CHAR_INFO[m_size];
    m_startRect = {0, 0, m_displayRect.w, m_displayRect.h};
    m_startCurs = info.dwCursorPosition;

    ReadConsoleOutput(
        m_stdout,                            // handle
        m_startBuf,                          // buffer
        {m_displayRect.w, m_displayRect.h},  // sizeof buffer
        {0, 0},                              // start coordinates
        &m_startRect);                       // actual rect if not the same
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

uint8_t ConsoleWindows::getColorImpl(uint8_t fg, uint8_t bg)
{
    if (bg != CS_TRANSPARENT)
    {
        if (fg < 16 && bg < 16)
            return COLOR_TABLE[bg][fg];
        else
            return CS_WHITE;
    }
    else
    {
        if (fg < 16)
            return COLOR_TABLE[CS_BLACK][fg];
        else
            return CS_WHITE;
    }
}

BOOL WINAPI CtrlCallback(DWORD evt)
{
    switch (evt)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    {
        // generate an event to exit
        INPUT_RECORD inputRecord;
        inputRecord.EventType                        = KEY_EVENT;
        inputRecord.Event.KeyEvent.bKeyDown          = TRUE;
        inputRecord.Event.KeyEvent.dwControlKeyState = LEFT_CTRL_PRESSED;
        inputRecord.Event.KeyEvent.wVirtualKeyCode   = VK_C;

        DWORD eventsWritten;
        WriteConsoleInput(
            GetStdHandle(STD_INPUT_HANDLE),
            &inputRecord,
            1,
            &eventsWritten);

        // Skip it or pass it on to the default
        // handler. If this fails, it just won't
        // exit cleanly (reset the screen etc ...)
        return eventsWritten > 0 ? TRUE : FALSE;
    }
    }

    return FALSE;
}

void ZeroBufferMemory(CHAR_INFO *dest, size_t size)
{
    CHAR_INFO *end = dest + size;
    while (dest != end)
        *dest++ = NullChar;
}

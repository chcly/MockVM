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
#define VK_B 0x42
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

    m_supportsColor = true;

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
        if (rec.dwControlKeyState & LEFT_CTRL_PRESSED ||
            rec.dwControlKeyState & RIGHT_CTRL_PRESSED)
        {
            if (rec.wVirtualKeyCode == VK_C ||
                rec.wVirtualKeyCode == VK_Z)
            {
                rc = CCS_FORCE_EXIT;
            }
        }
        else
        {
            switch (rec.wVirtualKeyCode)
            {
            case VK_DOWN:
            case VK_F10:
            case VK_F11:
                if (rec.dwControlKeyState & SHIFT_PRESSED)
                    rc = CCS_STEP_OUT;
                else
                    rc = CCS_STEP;
                break;
            case VK_B:
            case VK_F9:
                rc = CCS_ADD_BREAKPOINT;
                break;
            case VK_C:
            case VK_F5:
                rc = CCS_CONTINUE;
                break;
            case VK_Q:
            case VK_ESCAPE:
                rc = CCS_QUIT;
                break;
            case VK_R:
                rc = CCS_RESTART;
                break;
            default:
                break;
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

int ConsoleWindows::nextCommand()
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
        // continue to filter events.
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
        inputRecord.Event.KeyEvent.wVirtualKeyCode   = VK_Z;

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

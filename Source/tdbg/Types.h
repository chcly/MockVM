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
#ifndef _Types_h_
#define _Types_h_

#include <stdint.h>
#include "Declarations.h"

enum ColorSpace
{
    CS_BLACK = 0,
    CS_DARKBLUE,
    CS_DARKGREEN,
    CS_DARKCYAN,
    CS_DARKRED,
    CS_DARKMAGENTA,
    CS_DARKYELLOW,
    CS_LIGHT_GREY,
    CS_GREY,
    CS_BLUE,
    CS_GREEN,
    CS_CYAN,
    CS_RED,
    CS_MAGENTA,
    CS_YELLOW,
    CS_WHITE,
    CS_TRANSPARENT,
    CS_COLOR_MAX
};

enum ConsoleCtrlStatus
{
    CCS_NO_INPUT = 0,
    CCS_QUIT,
    CCS_FORCE_EXIT,
    CCS_STEP,
    CCS_STEP_OUT,
    CCS_ADD_BREAKPOINT,
    CCS_CONTINUE,
    CCS_RESTART,
    CCS_REDRAW,
    CCS_RESIZE,
    CCS_CLICKED
};

struct ConsolePoint
{
    int16_t x, y;
};

struct ConsoleRect
{
    int16_t x, y, w, h;

    inline int16_t right()
    {
        return x + w;
    }

    inline int16_t bottom()
    {
        return y + h;
    }

    inline int16_t cx()
    {
        return x + (w >> 1);
    }

    inline int16_t cy()
    {
        return y + (h >> 1);
    }
};

class ConsoleListener
{
public:
    virtual ~ConsoleListener()
    {
    }

    virtual int mouseClicked(const ConsolePoint &pt, int bt) = 0;
};

enum DebugFlags
{
    DF_BREAK = 0x01
};

struct DebugInstruction
{
    uint8_t         flags;
    str_t           value;
    ExecInstruction inst;
};

using DebugInstructions = std::vector<DebugInstruction>;

#endif  // !_Types_h_

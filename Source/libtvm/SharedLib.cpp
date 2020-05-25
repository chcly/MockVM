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
#include "SharedLib.h"
#include "Program.h"

uint8_t prog_get_register8(tvmregister_t regi, uint8_t reg)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        return ctx[reg].b[0];
    }
    return 0;
}

uint16_t prog_get_register16(tvmregister_t regi, uint8_t reg)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        return ctx[reg].w[0];
    }
    return 0;
}

uint32_t prog_get_register32(tvmregister_t regi, uint8_t reg)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        return ctx[reg].l[0];
    }
    return 0;
}

uint64_t prog_get_register64(tvmregister_t regi, uint8_t reg)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        return ctx[reg].x;
    }
    return 0;
}

void prog_set_register8(tvmregister_t regi, uint8_t reg, uint8_t v)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        ctx[reg].b[0] = v;
    }
}

void prog_set_register16(tvmregister_t regi, uint8_t reg, uint16_t v)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        ctx[reg].w[0] = v;
    }
}

void prog_set_register32(tvmregister_t regi, uint8_t reg, uint32_t v)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        ctx[reg].l[0] = v;
    }
}

void prog_set_register64(tvmregister_t regi, uint8_t reg, uint64_t v)
{
    if (regi && reg >= 0 && reg < 10)
    {
        Register *ctx = (Register *)regi;
        ctx[reg].x    = v;
    }
}

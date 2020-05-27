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
#ifndef _SharedLib_h_
#define _SharedLib_h_

#define SYM_API extern "C"

#ifdef _WIN32
#define SYM_EXPORT __declspec(dllexport)
#define SYM_LOCAL
#else
#define SYM_EXPORT __attribute__((__visibility__("default")))
#define SYM_LOCAL __attribute__((__visibility__("hidden")))
#endif  //  _WIN32

#include <stdint.h>

typedef struct _register* tvmregister_t;

SYM_API SYM_LOCAL uint8_t  prog_get_register8(tvmregister_t regi, uint8_t reg);
SYM_API SYM_LOCAL uint16_t prog_get_register16(tvmregister_t regi, uint8_t reg);
SYM_API SYM_LOCAL uint32_t prog_get_register32(tvmregister_t regi, uint8_t reg);
SYM_API SYM_LOCAL uint64_t prog_get_register64(tvmregister_t regi, uint8_t reg);
SYM_API SYM_LOCAL void     prog_set_register8(tvmregister_t regi, uint8_t reg, uint8_t v);
SYM_API SYM_LOCAL void     prog_set_register16(tvmregister_t regi, uint8_t reg, uint16_t v);
SYM_API SYM_LOCAL void     prog_set_register32(tvmregister_t regi, uint8_t reg, uint32_t v);
SYM_API SYM_LOCAL void     prog_set_register64(tvmregister_t regi, uint8_t reg, uint64_t v);

#endif  //_SharedLib_h_

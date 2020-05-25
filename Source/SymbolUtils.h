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
#ifndef _SymbolUtils_h_
#define _SymbolUtils_h_

#include <stdint.h>
#include "Declarations.h"

// To bind external code to the binary.
// 1. Build a shared library.
// 2. Export the symbol *module_name*_init()
// 3. Link it to the executable  with tcom -l module_name.
// 4. The writer should check to see if the symbol is
//    in the shared library.
// 5. If the symbol is matched, store a reference to the
//    module in the symbol table.
// 6. Mark the symbol as IF_SYMU, and add the symbol name
//    to the string table.
// 7. Store the index in the string table, as the argument
//    to the call.
// 7. When loading, search the symbol table for modules.
// 8. Reload the modules and attempt to resolve callbacks
//    for symbols marked as IF_SYMU.
#ifdef _WIN32
#define SYM_EXPORT extern "C" __declspec(dllexport)
#else
#define SYM_EXPORT extern "C" __attribute__((__visibility__("default")))
#endif  //  _WIN32

typedef void* LibHandle;
typedef void* LibSymbol;

extern LibHandle LoadSharedLibrary(const str_t& modname, const str_t& moddir);
extern void      UnloadSharedLibrary(LibHandle handle);
extern LibSymbol GetSymbolAddress(LibHandle handle, const str_t& symname);
extern void      FindExecutableDirectory(str_t& dest);
extern bool      IsModulePresent(const str_t& modname, const str_t& moddir);


#endif  //_SymbolUtils_h_

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
#include "SymbolUtils.h"
#include "Declarations.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

LibHandle LoadSymbolLibrary(const char* libname)
{
    HMODULE lib = LoadLibrary(libname);
    if (!lib)
    {
        printf("Failed to load %s\n", libname);
        return nullptr;
    }
    return (LibHandle)lib;
}

void UnloadSymbolLibrary(LibHandle handle)
{
    if (handle)
        FreeLibrary((HMODULE)handle);
}

LibSymbol GetSymbolAddress(LibHandle handle, const char* symname)
{
    if (handle)
        return GetProcAddress((HMODULE)handle, symname);
    return nullptr;
}


#else

#include <dlfcn.h>
LibHandle LoadSymbolLibrary(const char* libname)
{
    str_t modname = libname;
    modname       = "lib" + modname + ".so";
    LibHandle lib = (LibHandle)dlopen(modname.c_str(), RTLD_LAZY);
    if (!lib)
    {
        modname = "./" + modname;
        lib = (LibHandle)dlopen(modname.c_str(), RTLD_LAZY);
        if (!lib)
        {
            printf("Failed to load %s\n", modname.c_str());
            return nullptr;
        }
    }
    return lib;
}

void UnloadSymbolLibrary(LibHandle handle)
{
    if (handle)
        dlclose((void*)handle);
}

LibSymbol GetSymbolAddress(LibHandle handle, const char* symname)
{
    if (handle)
        return dlsym((void*)handle, symname);
    return nullptr;
}
#endif

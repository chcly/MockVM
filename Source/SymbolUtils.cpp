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
#include <stdio.h>
#include <stdlib.h>
#include "Declarations.h"

#ifdef _WIN32
#include <windows.h>

LibHandle LoadSharedLibrary(const char* libname)
{
    str_t absPath;
    FindExecutableDirectory(absPath);

    if (absPath.empty())
    {
        // FindExecutableDirectory reports an error
        return nullptr;
    }

    // modules are stored in tvm_dir / lib for now.
    absPath += "/lib/";
    absPath += libname;
    absPath += ".dll";

    HMODULE lib = LoadLibrary(absPath.c_str());
    if (!lib)
    {
        printf("failed to find '%s'\n", absPath.c_str());
        return nullptr;
    }
    return (LibHandle)lib;
}

void UnloadSharedLibrary(LibHandle handle)
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

void FindExecutableDirectory(str_t& dest)
{
    dest.clear();
    HMODULE mod = GetModuleHandle(NULL);
    if (mod)
    {
        char  buf[270];
        DWORD sz = GetModuleFileName(mod, buf, 270);
        if (sz < 270)
        {
            buf[sz] = 0;
            dest    = str_t(buf, sz);

            size_t pos = dest.find_last_of('\\');
            if (pos != str_t::npos)
                dest = dest.substr(0, pos);
        }
        else
        {
            printf("failed to get the executable path.\n");
        }
    }
    else
    {
        printf("failed to get the module handle.\n");
    }
}

#else

#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

LibHandle LoadSharedLibrary(const char* libname)
{
    str_t absPath;
    FindExecutableDirectory(absPath);

    if (absPath.empty())
    {
        // FindExecutableDirectory reports an error
        return nullptr;
    }

    // modules are stored in tvm_dir / lib for now.
    absPath += "/lib/lib";
    absPath += libname;
    absPath += ".so";

    LibHandle lib = (LibHandle)dlopen(absPath.c_str(), RTLD_LAZY);
    if (!lib)
    {
        printf("failed to find '%s'\n", absPath.c_str());
        return nullptr;
    }
    return lib;
}

void UnloadSharedLibrary(LibHandle handle)
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

void FindExecutableDirectory(str_t& dest)
{
    dest.clear();
    char    buf[270];
    ssize_t sz = readlink("/proc/self/exe", buf, 270);
    if (sz > 0 && sz < 270)
    {
        buf[sz] = 0;
        dest    = str_t(buf, sz);

        size_t pos = dest.find_last_of('/');
        if (pos != str_t::npos)
            dest = dest.substr(0, pos);
    }
    else
    {
        printf("failed to get the executable path.\n");
    }
}

#endif

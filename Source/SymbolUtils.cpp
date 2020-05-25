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
#include <sys/stat.h>

void MakeModulePath(str_t& absPath, const str_t& modname, const str_t& moddir);


#ifdef _WIN32
#include <windows.h>


LibHandle LoadSharedLibrary(const str_t& modname, const str_t& moddir)
{
    str_t absPath;
    MakeModulePath(absPath, modname, moddir);

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

LibSymbol GetSymbolAddress(LibHandle handle, const str_t& symname)
{
    if (handle)
        return GetProcAddress((HMODULE)handle, symname.c_str());
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

LibHandle LoadSharedLibrary(const str_t& modname, const str_t& moddir)
{
    str_t absPath;
    MakeModulePath(absPath, modname, moddir);

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

LibSymbol GetSymbolAddress(LibHandle handle, const str_t& symname)
{
    if (handle)
        return dlsym((void*)handle, symname.c_str());
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


bool IsModulePresent(const str_t& modname, const str_t& moddir)
{
    str_t absPath;
    MakeModulePath(absPath, modname, moddir);

    struct stat _st;
    return stat(absPath.c_str(), &_st) == 0;
}

void MakeModulePath(str_t& absPath, const str_t& modname, const str_t& moddir)
{
    absPath.clear();

    absPath += moddir;
#ifdef _WIN32
    absPath += "\\lib\\lib";
    absPath += modname;
    absPath += ".dll";
#else
    absPath += "/lib/liblib";
    absPath += modname;
    absPath += ".so";
#endif
}

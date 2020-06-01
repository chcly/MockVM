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
#include <sys/stat.h>
#include "Declarations.h"

#define MAX_PATH_LEN 270

void MakeModulePath(str_t& absPath, const str_t& modname, const str_t& moddir);
void MakeModulePath(str_t& dest, char* buffer, size_t sz);

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

void FindModuleDirectory(str_t& dest)
{
    dest.clear();
    HMODULE mod = GetModuleHandle(NULL);
    if (mod)
    {
        char  buf[MAX_PATH_LEN + 1];
        DWORD sz = GetModuleFileName(mod, buf, MAX_PATH_LEN);
        if (sz > 0 && sz < MAX_PATH_LEN)
            MakeModulePath(dest, buf, sz);
        else
            printf("failed to get the executable path.\n");
    }
    else
        printf("failed to get the module handle.\n");
}

#else

#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>

LibHandle LoadSharedLibrary(const str_t& modname, const str_t& moddir)
{
    str_t absPath;
    MakeModulePath(absPath, modname, moddir);
    LibHandle lib = (LibHandle)dlopen(absPath.c_str(), RTLD_LOCAL | RTLD_LAZY);
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
    LibSymbol rval = nullptr;
    if (handle)
        rval = dlsym((void*)handle, symname.c_str());
    return rval;
}

#ifdef __APPLE__
#include <mach-o/dyld.h>

void FindModuleDirectory(str_t& dest)
{
    dest.clear();
    char buf[MAX_PATH_LEN + 1] = {};

    uint32_t sz = MAX_PATH_LEN;
    int      rc = _NSGetExecutablePath(buf, &sz);
    if (sz > 0 && sz <= MAX_PATH_LEN && rc == 0)
        MakeModulePath(dest, buf, sz);
    else
        printf("failed to get the executable path.\n");
}

#else

void FindModuleDirectory(str_t& dest)
{
    dest.clear();
    char    buf[MAX_PATH_LEN + 1];
    ssize_t sz = readlink("/proc/self/exe", buf, MAX_PATH_LEN);
    if (sz > 0 && sz < MAX_PATH_LEN)
        MakeModulePath(dest, buf, sz);
    else
        printf("failed to get the executable path.\n");
}

#endif  // __APPLE__
#endif  // _WIN32

bool IsModulePresent(const str_t& modname, const str_t& moddir)
{
    str_t absPath;
    MakeModulePath(absPath, modname, moddir);

    struct stat _st;
    return stat(absPath.c_str(), &_st) == 0;
}

void MakeModulePath(str_t& absPath, const str_t& modname, const str_t& moddir)
{
    // Maintain control of the loaded module by allowing only absolute
    // paths to be specified. It's ether where it should be or not found at all.
    absPath.clear();

    absPath += moddir;
#ifdef _WIN32
    absPath += "\\";
    absPath += modname;
    absPath += ".dll";
#elif __APPLE__
    absPath += "lib";
    absPath += modname;
    absPath += ".dylib";
#else
    absPath += "lib";
    absPath += modname;
    absPath += ".so";
#endif
}

void MakeModulePath(str_t& dest, char* buf, size_t sz)
{
    int s = (int)sz;
    if (buf[s] == '/' || buf[s] == '\\')
        s--;

    while (s > 0)
    {
        if (buf[s] == '/' || buf[s] == '\\')
            break;
        --s;
    }

    dest = str_t(buf, s);
#ifdef _WIN32
    dest += "\\lib\\";
#else
    dest += "/lib/";
#endif
}


void DisplayModulePath(void)
{
    str_t path;
    FindModuleDirectory(path);
    puts((path).c_str());
}

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
#include "Console.h"
#include "Debugger.h"
#include "SymbolUtils.h"

void usage(void);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    str_t file, mod;
    int   i;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            file = argv[i];
        else
        {
            char ch = argv[i][1];
            if (ch == 'h')
            {
                usage();
                return 0;
            }
        }
    }

    if (file.empty())
    {
        usage();
        puts("no input file");
        return 0;
    }

    FindModuleDirectory(mod);
    Debugger prog(mod, file);
    if (prog.load(file.c_str()) != PS_OK)
        return 1;

    return prog.debug();
}

void usage(void)
{
    puts("tdbg <options> <program_path>\n");
    puts("      options:");
    puts("          -h display this message.\n");
}

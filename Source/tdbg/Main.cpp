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

#ifndef _WIN32
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
volatile static sig_atomic_t exit_hook = 0;

void exithook(int)
{
    putchar('\n');
    exit_hook = 1;
    fflush(stdout);
}

void exit_program(int pid)
{
    kill(pid, SIGUSR1);
    kill(pid, SIGUSR2);
}
#endif

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

#ifdef _WIN32

    FindModuleDirectory(mod);
    Debugger prog(mod, file);
    if (prog.load(file.c_str()) != PS_OK)
        return 1;

    return prog.debug();

#else

    signal(SIGUSR1, exithook);
    signal(SIGUSR2, exithook);

    int p[2];
    if (pipe(p) == -1)
    {
        puts("failed to create pipe");
        return -1;
    }

    int pid = fork(), rc = 0;
    if (pid == -1)
    {
        puts("failed to fork process");
        return 1;
    }
    else if (pid == 0)
    {
        if (dup2(p[1], 1) == -1)
        {
            puts("failed to duplicate file descriptor");
            exit_program(pid);
            return 1;
        }

        stdout = fdopen(0, "wb");

        while (!exit_hook)
            usleep(1);
        puts("exited.\n");
        rc = 0;
    }
    else
    {
        FILE *pstd = stdout;
        if (dup2(p[1], 1) == -1)
        {
            puts("failed to duplicate file descriptor");
            exit_program(pid);
            return 1;
        }
        stdout = fdopen(0, "wb");

        FindModuleDirectory(mod);
        Debugger prog(mod, file);
        if (prog.load(file.c_str()) != PS_OK)
        {
            exit_program(pid);
            return 1;
        }
        else
        {
            rc = prog.debug();
            exit_program(pid);
        }

        stdout = pstd;
    }
    return rc;
#endif
}

void usage(void)
{
    puts("tdbg <options> <program_path>\n");
    puts("      options:");
    puts("          -h display this message.\n");
}

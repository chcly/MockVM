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
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include "Program.h"

using namespace std;

void usage(void);

struct ProgramInfo
{
    bool   time;
    string file;
};

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    ProgramInfo ctx = {};
    int         i;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            ctx.file = argv[i];
        else
        {
            char ch = argv[i][1];
            if (ch == 't')
                ctx.time = true;
            else if (ch == 'h')
            {
                usage();
                return 0;
            }
        }
    }

    if (ctx.file.empty())
    {
        usage();
        cout << "no input file\n";
        return 0;
    }

    Program prog;
    if (prog.load(ctx.file.c_str()) != PS_OK)
    {
        printf("failed to load %s\n", ctx.file.c_str());
        return -1;
    }

    int rc = 0;
    if (ctx.time)
    {
        _RELITAVE_TIME_CHECK_BEGIN
        rc = prog.launch();
        _RELITAVE_TIME_CHECK_END;
    }
    else
        rc = prog.launch();
    return rc;
}

void usage(void)
{
    cout << "tvm <options> <program_path>\n\n";
    cout << "    options:\n\n";
    cout << "        -h display this message\n";
    cout << "        -t display execution time\n";
}

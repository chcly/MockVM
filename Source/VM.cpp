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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include "BinaryReader.h"
#include "Program.h"


using namespace std;

void usage(void);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    vector<string> files;

    int i;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            files.push_back(argv[i]);
    }

    Program prog;
    if (!files.empty())
        prog.load(files.back().c_str());
    return prog.launch();
}


void usage(void)
{
}


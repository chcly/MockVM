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
#include "BlockReader.h"
#include "BinaryWriter.h"
#include "Parser.h"
#include "Declarations.h"

using namespace std;

typedef vector<string> strvec_t;

struct ProgramInfo
{
    string   output;
    strvec_t files;
    strvec_t modules;
};

void usage(void);


int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage();
        return 0;
    }

    ProgramInfo ctx = {};
    int    i;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
            case 'h':
                usage();
                exit(0);
                break;
            case 'o':
                if (i + 1 < argc)
                    ctx.output = (argv[++i]);
                break;
            case 'l':
                if (i + 1 < argc)
                    ctx.modules.push_back((argv[++i]));
                break;
            default:
                break;
            }
        }
        else
            ctx.files.push_back(argv[i]);
    }

    if (ctx.output.empty())
    {
        usage();
        cout << "missing output file.\n";
        return PS_ERROR;
    }

    BinaryWriter w;

    // Stage 1 - compile files
    for (string file : ctx.files)
    {
        Parser p;
        if (p.parse(file.c_str()) != PS_OK)
            return PS_ERROR;


        // First, merge the labels
        w.mergeLabels(p.getLabels()); 


        // Second, merge the instructions.
        // The labels should be stored first so instructions 
        // with label names can be filtered in one pass.
        w.mergeInstructions(p.getInstructions());


        // This has not been tested on multiple files yet.
        break;
    }

    // Stage 2 - resolve symbols
    if (w.resolve(ctx.modules) != PS_OK)
        return PS_ERROR;

    // Stage 3 - write the binary
    if (w.open(ctx.output.c_str()) != PS_OK)
        return PS_ERROR;
    if (w.writeHeader()!= PS_OK)
        return PS_ERROR;
    if (w.writeSections() != PS_OK)
        return PS_ERROR;
    return 0;
}


void usage(void)
{
    cout << "tcom <options> <input file>\n\n";
    cout << "    options:\n\n";
    cout << "        -h show this message.\n";
    cout << "        -o output file.\n";
    cout << "        -l link library.\n";
}

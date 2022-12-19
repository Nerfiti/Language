#include <cassert>
#include <cstdio>

#include "logs.hpp"
#include "frontend.hpp"

int main(const int argc, const char *argv[])
{
    initLog();
    
    const char *filename = (argc == 2) ? argv[1] : "TestProgram.lang";
    FILE *program = fopen(filename, "r");

    execute(program);

    assert(!fclose(program));
    closeLog();
}
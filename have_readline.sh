#!/bin/bash

# This script is called from Makefile, where the compiler and linker options are
# adjusted according to the output.

# We run a simple test to see if the system we're running on has readline
# library or not. We try to compile a simple program that uses readline. If it
# succeeds, we print "yes" to stdout. Otherwise we don't print anything.

# Makefile passes the compiler it's using.  Otherwise default to gcc.
CC=${1:-gcc}

{
    
$CC -x c -o a.exe - -lreadline << EOF
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
int main(void)
{
    (void) rl_initialize();
    return 0;
}
EOF
    
} >/dev/null 2>&1 && ./a.exe && echo yes

# clean up our mess
rm -f a.exe

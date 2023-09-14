
#include "daec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>

#define DESH_VERSION "0.1"

int main(int argc, char **argv)
{
    if (strcmp(DE_VERSION, de_version()) != 0)
    {
        fprintf(stderr, "ERROR: Library version mismatch:\n\tdaec.h: %s\n\tlibdaec.so: %s\n", DE_VERSION, de_version());
        return EXIT_FAILURE;
    }

    for (int i=1; i<argc; ++i)
    {
        if (strcmp(argv[i], "-v") == 0)
        {
            printf("DataEcon SHell (desh) %s\nusing DataEcon Library (libdaec) %s\n", DESH_VERSION, de_version());
            return EXIT_SUCCESS;
        }
        printf("argv[%d] = %s\n", i, argv[i]);
    };

    printf("DataEcon SHell\n   !!! Under construction, come back later !!!\n");
    return EXIT_SUCCESS;
}

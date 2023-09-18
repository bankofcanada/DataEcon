

#include <string>
#include <iostream>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "daec.h"

using namespace std;

void print_usage(const char *program)
{
    std::cerr << "Usage: " << program << " infile.daec out.data.csv out.manifest.csv\n";
    std::cerr << "    `outfile` should be just the basename of the output file.\n";
    std::cerr << "    Two .csv file will be written - outfile.data.csv and outfile.manifest.csv.\n";
    std::cerr << std::endl;
}

int print_de_error()
{
    char message[1024];
    int rc = de_error(message, sizeof message);
    std::cerr << message << std::endl;
    return rc;
}


int export_catalog(de_file de, obj_id_t pid)
{
    de_search search;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    de_file de;
    if (de_open_readonly(argv[1], &de) != DE_SUCCESS)
    {
        return print_de_error();
    }

    // FILE *D = stdout, *M = NULL;
    // if (argc >= 2)
    // {
    //     D = open(argv[2], "w");
    //     if (D == NULL)
    //     {
    //         fprintf(stderr, "Failed to open output data file.");
    //         return EXIT_FAILURE;
    //     }
    //     if (argc > 2)
    //     {
    //         M = open(argv[3], "w");
    //         if (M == NULL)
    //         {
    //             fprintf(stderr, "Failed to open output manifest file.");
    //             return EXIT_FAILURE;
    //         }
    //     }
    // }

    return export_catalog(de, 0);
}

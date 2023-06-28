#include "daec.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    de_file de;
    int rc;

    rc = de_open("example.daec", &de);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to open the file.\n");
        return EXIT_FAILURE;
    }

    char message[] = "Hello World";
    rc = de_store_scalar(de, 0, "message", type_string, freq_none,
                         sizeof message + 1, message, NULL);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to write the message\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    obj_id_t id;
    rc = de_find_fullpath(de, "/message", &id);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to find the message.\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    scalar_t scalar;
    rc = de_load_scalar(de, id, &scalar);
    if (rc != DE_SUCCESS)
    {
        fprintf(stderr, "Failed to read the message.\n");
        de_close(de);
        return EXIT_FAILURE;
    }

    printf("%s\n", (char *)scalar.value);

    de_close(de);
    return EXIT_SUCCESS;
}

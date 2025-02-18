
#include "daec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

de_file de;
char msg[1024];
#define CHECK(rc)                                         \
    if (rc)                                               \
    {                                                     \
        de_error_source(msg, sizeof msg - 1);             \
        printf("%s:%d => %s\n", __FILE__, __LINE__, msg); \
        de_close(de);                                     \
        exit(EXIT_FAILURE);                               \
    }

int main(void)
{
    int rc;

    CHECK(de_open("prof.daec", &de));

    obj_id_t scalars;
    rc = de_new_catalog(de, 0, "scalars", &scalars);
    CHECK(rc);

    double x;
    for (int i = 1; i <= 100000; ++i)
    {
        x = i + 1;
        snprintf(msg, 1023, "x%d", i);

        rc = de_store_scalar(de, scalars, msg, type_float, freq_none, sizeof(x), &x, NULL);
        CHECK(rc)

        snprintf(msg, 1023, "x%d_name", i);
        rc = de_store_scalar(de, scalars, msg, type_string, freq_none, 1 + strlen(msg), msg, NULL);
        CHECK(rc)
    }

    obj_id_t series;
    rc = de_new_catalog(de, 0, "series", &series);
    CHECK(rc);

#   define VECLEN 400

    axis_id_t axis;
    rc = de_axis_plain(de, VECLEN, &axis);
    CHECK(rc);

    double xvals[VECLEN];
    for (int i = 1; i <= 100000; ++i)
    {
        xvals[i % VECLEN] = i + 1;
        snprintf(msg, 1023, "x%d", i);

        rc = de_store_tseries(de, series, msg, type_vector, type_float, freq_none, axis, sizeof(xvals), &xvals, NULL);
        CHECK(rc);
    }

    rc = de_close(de);
    CHECK(rc);

    return EXIT_SUCCESS;
}

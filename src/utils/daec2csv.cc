
#include <cstdio>
#include <string>
#include <complex>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "common.h"

#include "daec.h"

using namespace std;

static std::FILE *D = NULL;
static std::FILE *M = NULL;
static de_file de = NULL;

bool export_scalars = export_scalars;

void print_usage(const char *program)
{
    fprintf(stderr, "Usage: %s infile.daec out.data.csv out.manifest.csv\n", program);
    fprintf(stderr, "    `outfile` should be just the basename of the output file.\n");
    fprintf(stderr, "    Two .csv file will be written - outfile.data.csv and outfile.manifest.csv.\n");
    fprintf(stderr, "\n");
    fflush(stderr);
    return;
}

void print_header()
{
    if (M != NULL)
    {
        fprintf(M, "name,class,type,frequency\n");
    }
    if (D != NULL)
    {
        fprintf(D, "date,name,value\n");
    }
}

int get_object_name(const object_t &object, const char **obj_name)
{
    if (object.pid == 0)
    {
        *obj_name = object.name;
        return DE_SUCCESS;
    }
    else
        return de_get_object_info(de, object.id, obj_name, NULL, NULL);
}

int export_scalar(const object_t &object)
{
    scalar_t scalar;
    int rc = de_load_scalar(de, object.id, &scalar);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return rc;
    }

    const char *obj_name;
    rc = get_object_name(object, &obj_name);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return rc;
    }

    if (M != NULL)
    {
        fprintf(M, "\"%s\",%s,%s,%s\n", obj_name,
                _find_class_text(object.obj_class),
                _find_type_text(object.obj_type),
                _find_frequency_text(scalar.frequency));
    }

    if (D != NULL)
    {
        fprintf(D, "\"N/A\",\"%s\",", obj_name);
        print_value(D, object.obj_type, scalar.frequency, scalar.nbytes, scalar.value);
        fprintf(D, "\n");
    }
    return 0;
}

int export_series(const object_t &object)
{
    tseries_t tseries;
    int rc = de_load_tseries(de, object.id, &tseries);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return rc;
    }

    const char *obj_name;
    rc = get_object_name(object, &obj_name);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return rc;
    }

    std::string name(obj_name);

    if (M != NULL)
    {
        fprintf(M, "\"%s\",%s,%s,%s\n", obj_name,
                _find_class_text(object.obj_class),
                _find_type_text(object.obj_type),
                _find_frequency_text(tseries.axis.frequency));
    }

    if (D != NULL)
    {
        frequency_t freq = tseries.axis.frequency;
        date_t first_date = tseries.axis.first;
        int64_t elbytes = tseries.nbytes / tseries.axis.length;
        type_t eltype;
        frequency_t elfreq;
        const int8_t * valptr = (const int8_t *)tseries.value;
        (void)de_unpack_eltype(tseries.eltype, &eltype, &elfreq);
        for (auto i = 0; i < tseries.axis.length; ++i)
        {
            date_t date = first_date + i;
            fprintf(D, "\"");
            print_date(D, freq, sizeof date, &date);
            fprintf(D, "\",\"%s\",", obj_name);
            print_value(D, eltype, elfreq, elbytes, valptr + elbytes * i);
            fprintf(D, "\n");
        }
    }
    return 0;
}

int export_catalog(obj_id_t pid)
{
    int rc;
    object_t object;
    de_search search;
    rc = de_list_catalog(de, pid, &search);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return rc;
    }
    rc = de_next_object(search, &object);
    while (rc == DE_SUCCESS)
    {
        switch (object.obj_class)
        {
        case class_catalog:
            (void)export_catalog(object.id);
            break;
        case class_scalar:
            (void)export_scalar(object);
            break;
        case class_tseries:
            (void)export_series(object);
            break;
        default:
            print_error("Cannot process object class %s(%d)", _find_class_text(object.obj_class), object.obj_class);
        }
        rc = de_next_object(search, &object);
    }
    if (rc != DE_NO_OBJ)
    {
        print_de_error();
    }
    rc = de_finalize_search(search);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
    }
    return rc;
}

void close_all()
{
    if (M != NULL)
    {
        fclose(M);
        M = NULL;
    }
    if (D != NULL)
    {
        fclose(D);
        D = NULL;
    }
    if (de != NULL)
    {
        de_close(de);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (de_open_readonly(argv[1], &de) != DE_SUCCESS)
    {
        print_de_error();
        return EXIT_FAILURE;
    }

    if (argc >= 2)
    {
        D = fopen(argv[2], "w");
        if (D == NULL)
        {
            print_error("Failed to open file %s", argv[2]);
            close_all();
            return EXIT_FAILURE;
        }
        if (argc > 2)
        {
            M = fopen(argv[3], "w");
            if (M == NULL)
            {
                print_error("Failed to open file %s", argv[3]);
                close_all();
                return EXIT_FAILURE;
            }
        }
    }

    set_date_fmt(date_fmt_ymd);

    print_header();
    auto rc = export_catalog(0);
    close_all();

    return rc == DE_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

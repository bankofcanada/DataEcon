
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if defined HAVE_ZLIB
#include <zlib.h>
#endif

#include "common.h"

#include "daec.h"

static de_file de = NULL;

#if !defined ZLIB_H
typedef void *gzFile;
int gzputs(gzFile file, const char *message) { return 0; }
int gzclose(gzFile file) { return 0; }
int gzbuffer(gzFile file, unsigned size) { return 0; }
gzFile gzopen(const char *path, const char *mode)
{
    print_error("Cannot open compressed file - zlib library not available");
    return NULL;
}
#endif

struct outFile
{
    bool compressed;
    union F_gz
    {
        FILE *F;
        gzFile gz;
    } file;
} OutputFile;

static struct outFile M = {false, {NULL}};
static struct outFile D = {false, {NULL}};

void print_usage(const char *program)
{
    fprintf(stderr, "Usage: %s infile.daec out.data.csv out.manifest.csv\n", program);
    fprintf(stderr, "    `outfile` should be just the basename of the output file.\n");
    fprintf(stderr, "    Two .csv file will be written - outfile.data.csv and outfile.manifest.csv.\n");
    fprintf(stderr, "\n");
    fflush(stderr);
    return;
}

void write_line(struct outFile *F, const char *line)
{
    if (F->compressed && F->file.gz != NULL)
        gzputs(F->file.gz, line);
    else if (F->file.F != NULL)
        fputs(line, F->file.F);
}

void print_header()
{
    write_line(&M, "name,class,type,frequency\n");
    write_line(&D, "date,name,value\n");
}

int get_object_name(const object_t *object, const char **obj_name)
{
    if (object->pid == 0)
    {
        *obj_name = object->name;
        return DE_SUCCESS;
    }
    else
        return de_get_object_info(de, object->id, obj_name, NULL, NULL);
}

int export_scalar(const object_t *object)
{
    scalar_t scalar;
    int rc = de_load_scalar(de, object->id, &scalar);
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

    char buffer[4096];
    snprintf(buffer, sizeof buffer, "\"%s\",%s,%s,%s\n", obj_name,
             _find_class_text(object->obj_class),
             _find_type_text(object->obj_type),
             _find_frequency_text(scalar.frequency));
    write_line(&M, buffer);

    char value[1024];
    (void)snprintf_value(value, sizeof value, object->obj_type, scalar.frequency, scalar.nbytes, scalar.value);
    snprintf(buffer, sizeof buffer, "\"N/A\",\"%s\",%s\n", obj_name, value);
    write_line(&D, buffer);

    return 0;
}

int export_series(const object_t *object)
{
    tseries_t tseries;
    int rc = de_load_tseries(de, object->id, &tseries);
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

    char buffer[4096];
    snprintf(buffer, sizeof buffer, "\"%s\",%s,%s,%s\n", obj_name,
             _find_class_text(object->obj_class),
             _find_type_text(object->obj_type),
             _find_frequency_text(tseries.axis.frequency));
    write_line(&M, buffer);

    {
        frequency_t freq = tseries.axis.frequency;
        date_t date = tseries.axis.first;
        int64_t elbytes = tseries.nbytes / tseries.axis.length;
        type_t eltype;
        frequency_t elfreq;
        const int8_t *valptr = (const int8_t *)tseries.value;
        (void)de_unpack_eltype(tseries.eltype, &eltype, &elfreq);
        if (eltype == type_string)
        {
            print_error("Cannot handle series of eltype string");
            return -1;
        }
        char sdate[1024], sval[1024];
        for (int i = 0; i < tseries.axis.length; ++i, ++date)
        {
            snprintf_date(sdate, sizeof sdate, freq, sizeof date, &date);
            snprintf_value(sval, sizeof sval, eltype, elfreq, elbytes, valptr);
            snprintf(buffer, sizeof buffer, "\"%s\",\"%s\",%s\n", sdate, obj_name, sval);
            write_line(&D, buffer);
            date += 1;
            valptr += elbytes;
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
            (void)export_scalar(&object);
            break;
        case class_tseries:
            (void)export_series(&object);
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
    if (M.compressed && M.file.gz != NULL)
        gzclose(M.file.gz);
    else if (M.file.F != NULL)
        fclose(M.file.F);
    if (D.compressed && D.file.gz != NULL)
        gzclose(D.file.gz);
    else if (D.file.F != NULL)
        fclose(D.file.F);
    if (de != NULL)
        de_close(de);
}

bool open_file(struct outFile *F, const char *fname)
{
    F->compressed = (strstr(fname, ".gz") != NULL);
    if (F->compressed)
    {
        F->file.gz = gzopen(fname, "w");
        gzbuffer(F->file.gz, 64 * 1024);
    }
    else
    {
        F->file.F = fopen(fname, "w");
    }
    if (F->file.F == NULL)
    {
        print_error("Failed to open file %s for writing.", fname);
        return false;
    }
    return true;
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
        print_error("Failed to open file %s for reading.", argv[1]);
        return EXIT_FAILURE;
    }

    if (argc > 2)
    {

        if (!open_file(&D, argv[2]))
        {
            close_all();
            return EXIT_FAILURE;
        }
        if (argc > 3)
        {
            if (!open_file(&M, argv[3]))
            {
                close_all();
                return EXIT_FAILURE;
            }
        }
    }

    set_date_fmt(date_fmt_ymd);

    print_header();
    int rc = export_catalog(0);
    close_all();

    return rc == DE_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}

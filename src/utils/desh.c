
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
void add_history(char *command) {}
#endif

#include "daec.h"
#define DESH_VERSION "0.1"

#include "common.h"

char *strip(char *line);
char *repl_read_command(void);
void repl_execute(char *command);
void new_catalog(void);

void print_version(FILE *F)
{
    fprintf(F, "DataEcon SHell version (desh) %s using DataEcon Library version (libdaec) %s\n", DESH_VERSION, de_version());
}

void print_usage(FILE *F, const char *program)
{
    fprintf(F, "Usage: %s [options...] [file.daec]\n", program);
    fprintf(F, "If a filename is given it will be opened.\n");
    fprintf(F, "Options:\n");
    fprintf(F, "   -v or --version     : display version information and exit.\n");
    fprintf(F, "   -h or -? or --help  : display this help information and exit.\n");
    fprintf(F, "\n");
}

static char desh_prompt[] = "desh> ";

// static de_file work = NULL;
static de_file db = NULL;
static bool quit = false;

void signal_int_handler(int signal)
{
    print_error("signal %d\n", signal);
    if (db)
        de_close(db);
}

int main(int argc, char **argv)
{
    if (strcmp(DE_VERSION, de_version()) != 0)
    {
        print_error("ERROR: Library version mismatch:\n\tdaec.h: %s\n\tlibdaec.so: %s\n", DE_VERSION, de_version());
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i)
    {
        if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0))
        {
            print_version(stdout);
            return EXIT_SUCCESS;
        }
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-?") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        }
    };

    signal(SIGINT, signal_int_handler);

    int rc;
    for (int i = 1; i < argc; ++i)
    {
        if (db != NULL)
        {
            de_close(db);
            print_error("ERROR: only one file can be opened.\n");
            return EXIT_FAILURE;
        }
        rc = de_open(argv[i], &db);
        if (rc != DE_SUCCESS)
        {
            print_error("ERROR: failed to open file %s.\n", argv[i]);
            print_de_error();
            return EXIT_FAILURE;
        }
    }

    if (db == NULL)
    {
        rc = de_open_memory(&db);
        if (rc != DE_SUCCESS)
        {
            print_error("ERROR: Failed to open work database\n");
            print_de_error();
            return EXIT_FAILURE;
        }
    }

    print_version(stdout);
    fprintf(stdout, "   !!!  Under Construction  !!!\n");

    while (!quit)
    {
        char *line = repl_read_command();
        if (line == NULL)
            break;
        char *s = strip(line);
        if (*s)
        {
            add_history(s);
            repl_execute(s);
        }

        free(line);
    }

    fprintf(stdout, "\n");

    de_close(db);
    return EXIT_SUCCESS;
}

char *strip(char *line)
{
    char *s = line;
    while (isspace(*s))
        ++s;
    if (s[0] == '\0')
        return s;
    char *t = s + strlen(s) - 1;
    while (t > s && isspace(*t))
        --t;
    t[1] = '\0';
    return s;
}

char *repl_read_command(void)
{
#ifdef HAVE_READLINE
    printf("\n");
    return readline(desh_prompt);
#else
    char *line = malloc(2048);
    size_t len = 2048;
    printf("\n%s", desh_prompt);
    if (fgets(line, len, stdin) == NULL)
    {
        free(line);
        return NULL;
    }
    return line;
#endif
}

/****************************************************************************
 * TODO: this is very primitive parsing and interpreting of user input.
 *       we need a proper parser and interpreter for this repl.
 ****************************************************************************/

void _split_name(const char *name, obj_id_t *pid, const char **basename)
{
    char *r = strrchr(name, '/'); // find the last '/' in name
    if (r == NULL)                // no '/' in name
    {
        *pid = 0;
        *basename = name;
    }
    else if (r == name) // '/' is the first character, as in name="/blah-blah-blah"
    {
        *pid = 0;
        *basename = name + 1;
    }
    else // name="/blah/blah/blah" and r is pointing at the last '/'
    {
        // *basename points to the last "blah"
        *basename = r + 1;
        // *pid is the id of "/blah/blah"
        // name is readonly, so we can't just overwrite the last '/' with a '\0'.
        // So, we make a copy of it into our own buffer
        size_t len = r - name;
        char *parent_fullpath = malloc(len + 1);
        if (parent_fullpath == NULL)
        {
            print_error("Failed to allocate memory");
            *basename = NULL;
            *pid = -1;
            return;
        }
        memcpy(parent_fullpath, name, len);
        parent_fullpath[len] = 0;
        int rc = de_find_fullpath(db, parent_fullpath, pid);
        if (rc != DE_SUCCESS)
        {
            print_error("Failed to find the id of parent catalog %s", parent_fullpath);
            print_de_error();
            *basename = NULL;
            *pid = -1;
            free(parent_fullpath);
            return;
        }
        free(parent_fullpath);
    }
}

void new_catalog(void)
{
    char const *name = strtok(NULL, " ");
    if (name == NULL)
    {
        print_error("Missing name.");
        return;
    }
    const char *basename;
    obj_id_t pid;
    _split_name(name, &pid, &basename);
    if (pid < 0 || basename == NULL)
        return; // failed and error message is already printed
    if (DE_SUCCESS != de_new_catalog(db, pid, basename, NULL))
        print_de_error();
    return;
}

void new_scalar(void)
{
    char const *type_str = strtok(NULL, " ");
    if (type_str == NULL)
    {
        print_error("Expected type.");
        return;
    }
    type_t type = _find_type_code(type_str);
    if ((int)type < 0 || type > type_other_scalar)
    {
        print_error("\"%s\" is not a scalar type", type_str);
        return;
    }

    char const *name = strtok(NULL, " ");
    if (name == NULL)
    {
        print_error("Expected name.");
        return;
    }
    const char *basename;
    obj_id_t pid;
    _split_name(name, &pid, &basename);
    if (pid < 0 || basename == NULL)
        return;

    char const *equal = strtok(NULL, " ");
    if (equal == NULL || strcmp(equal, "=") != 0)
    {
        print_error("Expected = found \"%s\".", equal);
        return;
    }

    frequency_t freq = freq_none;
    char const *freq_str = "";
    if (type == type_date)
    {
        freq_str = strtok(NULL, " ");
        if (freq_str == NULL)
        {
            print_error("Expected frequency.");
            return;
        }
        freq = _find_frequency_code(freq_str);
        if ((int)freq < 0)
        {
            print_error("\"%s\" is not a frequency.", freq_str);
            return;
        }
    }

    char const *value = strtok(NULL, "\n");
    if (value == NULL)
    {
        print_error("Expected value.");
        return;
    }
    int64_t nbytes;
    void *val = malloc(100);
    if (val == NULL)
    {
        print_error("Memory allocation error.");
        return;
    }
    switch (type)
    {
    case type_integer:
    {
        nbytes = sizeof(int64_t);
        int ret = sscanf(value, "%" SCNi64, (int64_t *)(val));
        if (ret != 1)
        {
            print_error("Failed to parse an integer number from %s", value);
            nbytes = -1;
        }
        break;
    }
    case type_unsigned:
    {
        nbytes = sizeof(uint64_t);
        int ret = sscanf(value, "%" SCNu64, (uint64_t *)(val));
        if (ret != 1)
        {
            print_error("Failed to parse an integer number from %s", value);
            nbytes = -1;
        }
        break;
    }
    case type_float:
    {
        nbytes = sizeof(double);
        int ret = sscanf(value, "%lg", (double *)val);
        if (ret != 1)
        {
            print_error("Failed to parse floating point number from %s", value);
            nbytes = -1;
        }
        break;
    }
    case type_date:
    {
        nbytes = sizeof(date_t);
        if (freq == freq_none)
        {
            print_error("Frequency must not be \"none\"");
            nbytes = -1;
        }
        else if (freq == freq_unit)
        {
            int ret = sscanf(value, "%" SCNd64, (int64_t *)val);
            if (ret != 1)
            {
                print_error("Failed to parse an integer from %s", value);
                nbytes = -1;
            }
        }
        else if (freq == freq_daily || freq == freq_bdaily || (freq & freq_weekly) != 0)
        {
            int32_t Y;
            uint32_t M, D;
            int ret = sscanf(value, "%" SCNd32 "-%" SCNu32 "-%" SCNu32, &Y, &M, &D);
            if (ret != 3)
            {
                print_error("Failed to parse date in format YYYY-MM-DD from %s", value);
                nbytes = -1;
            }
            int rc = de_pack_calendar_date(freq, Y, M, D, (date_t *)val);
            if (rc != DE_SUCCESS)
            {
                print_de_error();
                nbytes = -1;
            }
        }
        else
        {
            int32_t Y;
            uint32_t P;
            int ret = sscanf(value, "%" SCNd32 "-%" SCNu32, &Y, &P);
            if (ret != 2)
            {
                print_error("Failed to parse date in format year-period from %s", value);
                nbytes = -1;
            }
            int rc = de_pack_year_period_date(freq, Y, P, (date_t *)val);
            if (rc != DE_SUCCESS)
            {
                print_de_error();
                nbytes = -1;
            }
        }
        break;
    }
    case type_string:
    {
        size_t len = strlen(value);
        if (value[0] != '"' || value[len - 1] != '"')
        {
            print_error("String value must be between \"");
            nbytes = -1;
            break;
        }
        free(val);
        val = malloc(len + 1);
        memcpy(val, value + 1, len - 2);
        ((char *)val)[len - 2] = 0;
        nbytes = unescape_string(val, len + 1, val);
        break;
    }
    default:
    {
        print_error("Not implemented.");
        nbytes = -1;
        break;
    }
    }

    if (nbytes >= 0)
    {
        int rc = de_store_scalar(db, pid, basename, type, freq, nbytes, val, NULL);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
        }
    }
    free(val);
}

void print_scalar(FILE *F, obj_id_t id)
{
    scalar_t scalar;
    int rc = de_load_scalar(db, id, &scalar);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    char val[1024];
    snprintf_value(val, sizeof val, scalar.object.obj_type, scalar.frequency, scalar.nbytes, scalar.value);
    if (scalar.object.obj_type == type_date)
        fprintf(F, "%s %s", _find_frequency_text(scalar.frequency), val);
    else
        fprintf(F, "%s", val);
}

void print_catalog(FILE *F, obj_id_t id)
{
    int64_t count;
    int rc;
    rc = de_catalog_size(db, id, &count);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    // const char *fullpath;
    // rc = de_get_object_info(db, id, &fullpath, NULL, NULL);
    // if (rc != DE_SUCCESS)
    // {
    //     print_de_error();
    //     return;
    // }
    if (count == 0)
        fprintf(F, "empty catalog");
    else
        fprintf(F, "catalog containing %" PRId64 " objects", count);
    return;
}

void print_object_summary(FILE *F, object_t *obj)
{
    switch (obj->obj_class)
    {
    case class_catalog:
        print_catalog(F, obj->id);
        break;
    case class_scalar:
        print_scalar(F, obj->id);
        break;
    case class_tseries:
    {
        tseries_t series;
        if (DE_SUCCESS != de_load_tseries(db, obj->id, &series))
        {
            print_de_error();
            return;
        }
        fprintf(F, "%s %s size %" PRId64, _find_frequency_text(series.axis.frequency),
                _find_type_text(obj->obj_type), series.axis.length);
        break;
    }
    case class_mvtseries:
    {
        mvtseries_t series;
        if (DE_SUCCESS != de_load_mvtseries(db, obj->id, &series))
        {
            print_de_error();
            return;
        }
        fprintf(F, "%s %s size %" PRId64 "×%" PRId64, _find_frequency_text(series.axis1.frequency),
                _find_type_text(obj->obj_type), series.axis1.length, series.axis2.length);
        break;
    }
    default:
        // can't get here
        break;
    }
}

void print_object(FILE *F, obj_id_t id, bool summary)
{
    object_t obj;
    int rc = de_load_object(db, id, &obj);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    if (summary)
    {
        print_object_summary(F, &obj);
        return;
    }

    switch (obj.obj_class)
    {
    case class_scalar:
        print_scalar(F, obj.id);
        return;
    case class_catalog:
        print_catalog(F, obj.id);
        return;
    case class_tseries:
    case class_mvtseries:
    default:
        print_error("Printing of class %s not implemented.", _find_class_text(obj.obj_class));
        return;
    }
}

void list_catalog(FILE *F)
{
    const char *name = strtok(NULL, " ");
    obj_id_t cat_id = 0;
    if (name != NULL)
        cat_id = find_object_id(db, name);
    if (cat_id < 0)
    {
        return;
    }

    de_search search;

    if (DE_SUCCESS != de_list_catalog(db, cat_id, &search))
    {
        print_de_error();
        return;
    }
    object_t obj;
    int rc = de_next_object(search, &obj);
    while (rc == DE_SUCCESS)
    {
        fprintf(F, "%s = ", obj.name);
        print_object(F, obj.id, true);
        fprintf(F, "\n");
        rc = de_next_object(search, &obj);
    }
    if (rc != DE_NO_OBJ)
        print_de_error();
    if (DE_SUCCESS != de_finalize_search(search))
        print_de_error();
}

void print_help(FILE *F)
{
    fprintf(F, "%s - %s\n", "help", "display this message");
    fprintf(F, "%s - %s\n", "version", "display version information");
    fprintf(F, "%s - %s\n", "quit", "stop reading and interpreting user input and exit");
    fprintf(F, "%s - %s\n", "list", "list objects in the database displaying summary information");
    fprintf(F, "%s - %s\n", "display name", "display full info about the named object");
    fprintf(F, "%s - %s\n", "delete name", "delete the named object");
    fprintf(F, "%s - %s\n", "open <file.daec>", "close the current database and open the database with the given filename");
    // fprintf(F, "%s - %s\n", "scalar type name = value", "create new scalar object of the given type, name and value");
    return;
}

void delete_object(void)
{
    const char *name = strtok(NULL, " ");
    if (name == NULL)
    {
        print_error("Expected object name.");
        return;
    }
    obj_id_t id = find_object_id(db, name);
    if (id >= 0)
    {
        if (de_delete_object(db, id) != DE_SUCCESS)
        {
            print_de_error();
        }
    }
    return;
}

void display(FILE *F)
{
    const char *name = strtok(NULL, " ");
    if (name == NULL)
    {
        print_error("Expected object name.");
        return;
    }
    obj_id_t id = find_object_id(db, name);
    if (id >= 0)
    {
        print_object(F, id, false);
        fprintf(F, "\n");
    }
}

void _junk(bool run)
{
    if (run)
    {
        char *junk = strtok(NULL, "\n");
        if (junk != NULL)
        {
            print_error("Unexpected junk after command: %s", junk);
        }
    }
}

void repl_execute(char *command_line)
{
    char *command = strtok(command_line, " ");
    bool maybe_junk = true;
    if (strcmp(command, "quit") == 0)
    {
        quit = true;
    }
    else if (strcmp(command, "help") == 0)
    {
        print_help(stdout);
    }
    else if (strcmp(command, "version") == 0)
    {
        print_version(stdout);
    }
    else if (strcmp(command, "scalar") == 0)
    {
        new_scalar();
        maybe_junk = false;
    }
    else if (strcmp(command, "catalog") == 0)
    {
        new_catalog();
    }
    else if (strcmp(command, "list") == 0)
    {
        list_catalog(stdout);
    }
    else if (strcmp(command, "delete") == 0)
    {
        delete_object();
    }
    else if (strcmp(command, "display") == 0)
    {
        display(stdout);
    }
    else
    {
        char *argument = strtok(NULL, "\n");
        if (argument == NULL)
            argument = "";
        print_error("Unknown command %s %s", command, argument);
    }
    _junk(maybe_junk);
}


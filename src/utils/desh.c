
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
char *repl_read_command();
void repl_execute(char *command);

void print_version(FILE *F)
{
    fprintf(F, "DataEcon SHell (desh) %s using DataEcon Library (libdaec) %s\n", DESH_VERSION, de_version());
}

const char *desh_prompt = "desh> ";

static de_file workdb = NULL;

void signal_int_handler(int signal)
{
    print_error("signal %d\n", signal);
    if (workdb)
        de_close(workdb);
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
        if (strcmp(argv[i], "-v") == 0)
        {
            print_version(stdout);
            return EXIT_SUCCESS;
        }
        printf("argv[%d] = %s\n", i, argv[i]);
    };

    print_version(stdout);
    fprintf(stdout, "   !!!  Under Construction  !!!\n");

    int rc = de_open_memory(&workdb);
    if (rc != DE_SUCCESS)
    {
        print_error("ERROR: Failed to open work database\n");
        print_de_error();
        return EXIT_FAILURE;
    }

    signal(SIGINT, signal_int_handler);

    while (true)
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

    de_close(workdb);
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

char *repl_read_command()
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

void new_scalar(void)
{
    char const *type_str = strtok(NULL, " ");
    if (type_str == NULL)
    {
        print_error("Expected type.");
        return;
    }

    // parse scalar type
    frequency_t freq = freq_none;
    type_t type = _find_type_code(type_str);
    if (type < 0)
    {
        type = type_date;
        freq = _find_frequency_code(type_str);
        if (freq < 0)
        {
            print_error("Scalar type %s not supported.", type_str);
            return;
        }
    }

    char const *name = strtok(NULL, " ");
    if (name == NULL)
    {
        print_error("First argument must be the scalar's name");
        return;
    }

    char const *equal = strtok(NULL, " ");
    if (equal == NULL || strcmp(equal, "=") != 0)
    {
        print_error("Expected = found %s", equal);
        return;
    }

    // parse scalar value
    char const *value = strtok(NULL, " ");
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
        if (freq == freq_daily || freq == freq_bdaily || (freq & freq_weekly) != 0)
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
    default:

        nbytes = -1;
        break;
    }

    if (nbytes >= 0)
    {
        int rc = de_store_scalar(workdb, 0, name, type, freq, nbytes, val, NULL);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
        }
    }
    free(val);
}

void print_scalar(obj_id_t id)
{
    scalar_t scalar;
    int rc = de_load_scalar(workdb, id, &scalar);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    char svalue[1024];
    snprintf_value(svalue, sizeof svalue, scalar.object.obj_type, scalar.frequency, scalar.nbytes, scalar.value);
    fprintf(stdout, "%s", svalue);
}

void print_object(obj_id_t id)
{
    object_t obj;
    int rc = de_load_object(workdb, id, &obj);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    switch (obj.obj_class)
    {
    case class_scalar:
        print_scalar(obj.id);
        return;
    default:
        print_error("Printing of class %d not implemented.", obj.obj_class);
        return;
    }
}

void list_database(void)
{
    de_search search;

    if (DE_SUCCESS != de_list_catalog(workdb, 0, &search))
    {
        print_de_error();
        return;
    }
    object_t obj;
    int ret = de_next_object(search, &obj);
    while (ret == DE_SUCCESS)
    {
        fprintf(stdout, "%s = ", obj.name);
        print_object(obj.id);
        fprintf(stdout, "\n");
        ret = de_next_object(search, &obj);
    }
    if (ret != DE_NO_OBJ)
        print_de_error();
    if (DE_SUCCESS != de_finalize_search(search))
        print_de_error();
}

void print_help(FILE *F)
{
    fprintf(F, "%s - %s\n", "help", "show this message");
    fprintf(F, "%s - %s\n", "version", "show version information");
    fprintf(F, "%s - %s\n", "list", "list work database");
    fprintf(F, "%s - %s\n", "display name", "display named object");
    fprintf(F, "%s - %s\n", "delete name", "delete named object");
    fprintf(F, "%s - %s\n", "scalar type name = value", "create new scalar object of the given type, name and value");
    return;
}

void repl_execute(char *command_line)
{
    char *command = strtok(command_line, " ");

    if (strcmp(command, "help") == 0)
    {
        print_help(stdout);
        return;
    }

    if (strcmp(command, "version") == 0)
    {
        print_version(stdout);
        return;
    }

    if (strcmp(command, "scalar") == 0)
    {
        new_scalar();
        return;
    }

    if (strcmp(command, "list") == 0)
    {
        list_database();
        char *junk = strtok(NULL, " ");
        if (junk != NULL)
        {
            print_error("Unexpected junk after command: %s", junk);
        }
        return;
    }

    if (strcmp(command, "delete") == 0)
    {
        char *name = strtok(NULL, " ");
        obj_id_t id;
        int rc = de_find_object(workdb, 0, name, &id);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
            return;
        }
        rc = de_delete_object(workdb, id);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
            return;
        }
        char *junk = strtok(NULL, " ");
        if (junk != NULL)
        {
            print_error("Unexpected junk after object name: %s", junk);
        }
        return;
    }

    if (strcmp(command, "display") == 0)
    {
        char *name = strtok(NULL, " ");
        obj_id_t id;
        int rc = de_find_object(workdb, 0, name, &id);
        if (rc == DE_SUCCESS)
        {
            print_object(id);
            fprintf(stdout, "\n");
            char *junk = strtok(NULL, " ");
            if (junk != NULL)
            {
                print_error("Unexpected junk after object name: %s", junk);
            }
            return;
        }
        else
        {
            print_de_error();
            return;
        }
    }

    fprintf(stdout, "   unknown command %s", command);
    char *argument = strtok(NULL, " ");
    while (argument != NULL)
    {
        fprintf(stdout, " %s", argument);
        argument = strtok(NULL, " ");
    }
    fprintf(stdout, "\n");
}

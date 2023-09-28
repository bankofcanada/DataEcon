#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
void add_history(char *command) {}
#endif

#include "daec.h"
#define DESH_VERSION "0.1"

char *strip(char *line);
char *repl_read_command();
void repl_execute(char *command);

void print_version(FILE *F)
{
    fprintf(F, "DataEcon SHell (desh) %s using DataEcon Library (libdaec) %s\n", DESH_VERSION, de_version());
}

const char *desh_prompt = "desh> ";

static de_file workdb = NULL;

void print_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    va_end(args);
}
void print_de_error()
{
    static char message[1024];
    de_error(message, sizeof message - 1);
    print_error(message);
}

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
    char *line = NULL;
    size_t len;
    printf("\n%s", desh_prompt);
    if (getline(&line, &len, stdin) < 0)
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

struct frequencies_map
{
    frequency_t freq_code;
    const char *freq_name;
};

struct frequencies_map FREQUENCIES_MAP[] = {
    {freq_daily, "daily"},
    {freq_bdaily, "bdaily"},
    {freq_weekly, "weekly"},
    {freq_weekly_mon, "weekly_mon"},
    {freq_weekly_tue, "weekly_tue"},
    {freq_weekly_wed, "weekly_wed"},
    {freq_weekly_thu, "weekly_thu"},
    {freq_weekly_fri, "weekly_fri"},
    {freq_weekly_sat, "weekly_sat"},
    {freq_weekly_sun, "weekly_sun"},
    {freq_monthly, "monthly"},
    {freq_quarterly, "quarterly"},
    {freq_quarterly_jan, "quarterly_jan"},
    {freq_quarterly_feb, "quarterly_feb"},
    {freq_quarterly_mar, "quarterly_mar"},
    {freq_quarterly_apr, "quarterly_apr"},
    {freq_quarterly_may, "quarterly_may"},
    {freq_quarterly_jun, "quarterly_jun"},
    {freq_quarterly_jul, "quarterly_jul"},
    {freq_quarterly_aug, "quarterly_aug"},
    {freq_quarterly_sep, "quarterly_sep"},
    {freq_quarterly_oct, "quarterly_oct"},
    {freq_quarterly_nov, "quarterly_nov"},
    {freq_quarterly_dec, "quarterly_dec"},
    {freq_halfyearly, "halfyearly"},
    {freq_halfyearly_jan, "halfyearly_jan"},
    {freq_halfyearly_feb, "halfyearly_feb"},
    {freq_halfyearly_mar, "halfyearly_mar"},
    {freq_halfyearly_apr, "halfyearly_apr"},
    {freq_halfyearly_may, "halfyearly_may"},
    {freq_halfyearly_jun, "halfyearly_jun"},
    {freq_halfyearly_jul, "halfyearly_jul"},
    {freq_halfyearly_aug, "halfyearly_aug"},
    {freq_halfyearly_sep, "halfyearly_sep"},
    {freq_halfyearly_oct, "halfyearly_oct"},
    {freq_halfyearly_nov, "halfyearly_nov"},
    {freq_halfyearly_dec, "halfyearly_dec"},
    {freq_yearly, "yearly"},
    {freq_yearly_jan, "yearly_jan"},
    {freq_yearly_feb, "yearly_feb"},
    {freq_yearly_mar, "yearly_mar"},
    {freq_yearly_apr, "yearly_apr"},
    {freq_yearly_may, "yearly_may"},
    {freq_yearly_jun, "yearly_jun"},
    {freq_yearly_jul, "yearly_jul"},
    {freq_yearly_aug, "yearly_aug"},
    {freq_yearly_sep, "yearly_sep"},
    {freq_yearly_oct, "yearly_oct"},
    {freq_yearly_nov, "yearly_nov"},
    {freq_yearly_dec, "yearly_dec"},
    {-1, NULL}};

int _find_frequency_code(const char *text)
{
    int i = 0;
    while (true)
    {
        struct frequencies_map *F = &FREQUENCIES_MAP[i];
        if (F->freq_name == NULL || strcmp(text, F->freq_name) == 0)
            return F->freq_code;
        i += 1;
    }
}
const char *_find_frequency_text(frequency_t freq)
{
    int i = 0;
    while (true)
    {
        struct frequencies_map *F = &FREQUENCIES_MAP[i];
        if (F->freq_code == -1 || F->freq_code == freq)
            return F->freq_name;
        i += 1;
    }
}

struct types_map
{
    type_t type_code;
    const char *type_name;
};
struct types_map TYPES_MAP[] = {
    {type_none, "none"},
    {type_integer, "integer"},
    {type_signed, "signed"},
    {type_unsigned, "unsigned"},
    {type_date, "date"},
    {type_float, "float"},
    {type_complex, "complex"},
    {type_string, "string"},
    {type_other_scalar, "other_scalar"},
    {type_vector, "vector"},
    {type_range, "range"},
    {type_tseries, "tseries"},
    {type_other_1d, "other_1d"},
    {type_matrix, "matrix"},
    {type_mvtseries, "mvtseries"},
    {type_other_2d, "other_2d"},
    {type_any, "any"},
    {-1, NULL}};

int _find_type_code(const char *text)
{
    int i = 0;
    while (true)
    {
        struct types_map *T = &TYPES_MAP[i];
        if (T->type_name == NULL || strcmp(text, T->type_name) == 0)
            return T->type_code;
        i += 1;
    }
}
const char *_find_type_text(type_t type)
{
    int i = 0;
    while (true)
    {
        struct types_map *T = &TYPES_MAP[i];
        if (T->type_code == -1 || T->type_code == type)
            return T->type_name;
        i += 1;
    }
}

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
        int ret = sscanf(value, "%li", (int64_t *)(val));
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
            int ret = sscanf(value, "%d-%u-%u", &Y, &M, &D);
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
            int ret = sscanf(value, "%d-%u", &Y, &P);
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
    scalar_t scal;
    int rc = de_load_scalar(workdb, id, &scal);
    if (rc != DE_SUCCESS)
    {
        print_de_error();
        return;
    }
    switch (scal.object.obj_type)
    {
    case type_integer:
    {
        if (scal.nbytes == 8)
            fprintf(stdout, "%ld", *(int64_t *)scal.value);
        else if (scal.nbytes == 4)
            fprintf(stdout, "%d", *(int32_t *)scal.value);
        else if (scal.nbytes == 2)
            fprintf(stdout, "%hd", *(int16_t *)scal.value);
        else if (scal.nbytes == 1)
            fprintf(stdout, "%hhd", *(int8_t *)scal.value);
        else
            fprintf(stdout, "%d", *(int *)scal.value);
        break;
    }
    case type_unsigned:
    {
        if (scal.nbytes == 8)
            fprintf(stdout, "%ld", *(uint64_t *)scal.value);
        else if (scal.nbytes == 4)
            fprintf(stdout, "%d", *(uint32_t *)scal.value);
        else if (scal.nbytes == 2)
            fprintf(stdout, "%hd", *(uint16_t *)scal.value);
        else if (scal.nbytes == 1)
            fprintf(stdout, "%hhd", *(uint8_t *)scal.value);
        else
            fprintf(stdout, "%d", *(unsigned *)scal.value);
        break;
    }
    case type_float:
    {
        if (scal.nbytes == 8)
            fprintf(stdout, "%lg", *(double *)scal.value);
        else if (scal.nbytes == 4)
            fprintf(stdout, "%g", *(float *)scal.value);
        else
            fprintf(stdout, "%lg", *(double *)scal.value);
        break;
    }
    case type_date:
    {
        if (scal.frequency == freq_daily || scal.frequency == freq_bdaily || (scal.frequency & freq_weekly) != 0)
        {
            int32_t Y;
            uint32_t M, D;
            if (DE_SUCCESS != de_unpack_calendar_date(scal.frequency, *(date_t *)scal.value, &Y, &M, &D))
            {
                print_de_error();
                return;
            }
            fprintf(stdout, "%s %d-%02u-%02u", _find_frequency_text(scal.frequency), Y, M, D);
        }
        else
        {
            int32_t Y;
            uint32_t P;
            if (DE_SUCCESS != de_unpack_year_period_date(scal.frequency, *(date_t *)scal.value, &Y, &P))
            {
                print_de_error();
                return;
            }
            fprintf(stdout, "%s %d-%02u", _find_frequency_text(scal.frequency), Y, P);
        }
        break;
    }
    default:
    {
        print_error("Printing of scalar type %d not implemented.", scal.object.obj_type);
        break;
    }
    }
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
        fprintf(stdout, "%s =", obj.name);
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

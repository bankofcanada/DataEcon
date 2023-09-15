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

int main(int argc, char **argv)
{
    if (strcmp(DE_VERSION, de_version()) != 0)
    {
        fprintf(stderr, "ERROR: Library version mismatch:\n\tdaec.h: %s\n\tlibdaec.so: %s\n", DE_VERSION, de_version());
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

void repl_execute(char *command_line)
{
    char *command = strtok(command_line, " ");
    fprintf(stdout, "   running command %s", command);
    char *argument = strtok(NULL, " ");
    while (argument != NULL)
    {
        fprintf(stdout, ", %s", argument);
        argument = strtok(NULL, " ");
    }
    fprintf(stdout, "\n");
}

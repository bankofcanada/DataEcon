
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "misc.h"

/* check if a file exists at the given path */
bool _isfile(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}

const char *_id2str(int64_t id)
{
    static char buffer[100];
    snprintf(buffer, 100, "id=%lld", (long long)id);
    return buffer;
}

const char *_pidnm2str(int64_t pid, const char *name)
{
    static char buffer[100];
    snprintf(buffer, 100, "pid=%lld,name='%s'", (long long)pid, name);
    return buffer;
}


#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "misc.h"
#include "error.h"

const char *de_version(void)
{
    return DE_VERSION;
}

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

/*
    pack a vector of strings into a contiguous memory buffer.
    this may be needed before writing an array of strings
    NOTES:
    * each string in the list must be '\0'-terminated.
    * we use '\0' character to separate the individual strings in the buffer.
    * `bufsize` must not be NULL. On exit it will contain the number of bytes
      in the packed representation.
    * If on entry `*bufsize` < 0, then `buffer` is not accessed, the necessary
      buffer size is calculated and written into `*bufsize`, and we return
      DE_SUCCESS.
    * If on entry 0 <= `*bufsize` < number of bytes needed, `buffer` is not
      accessed, the necessary buffer size is written into `*bufsize`, and
      return error code DE_SHORT_BUF.
    * If on entry `*bufsize` is sufficiently large, we write the packed
      representation into `buffer` (must not be NULL) and the number of bytes
      actually used in `*bufsize`.
*/
int de_pack_strings(const char **strvec, int64_t length, char *buffer, int64_t *bufsize)
{
    if (strvec == NULL || bufsize == NULL)
        return error(DE_NULL);

    int64_t bs = 0;
    for (int i = 0; i < length; ++i)
        bs += strlen(strvec[i]) + 1;

    if (*bufsize <= 0)
    {
        *bufsize = bs;
        return DE_SUCCESS;
    }

    if (*bufsize < bs)
    {
        *bufsize = bs;
        return error(DE_SHORT_BUF);
    }

    if (buffer == NULL)
    {
        return error(DE_NULL);
    }

    *bufsize = bs;
    char *p = buffer;
    for (int i = 0; i < length; ++i, ++p)
    {
        for (const char *s = strvec[i]; *s != '\0'; ++p, ++s)
            *p = *s;
        *p = '\0';
    }

    return DE_SUCCESS;
}

/* "unpack" a buffer of strings into a vector of '\0'- terminated strings
   this may be needed after reading an array of strings
   NOTES:
    * `strvec` must point to a vector of `length` pointers to char. The pointers
      will be populated with the addresses of the beginnings of the individual
      strings packed in `buffer`
    * no data is actually copied
    * if there aren't `length` strings within the first `bufsize` bytes of
      `buffer`, return DE_ARG
    * all pointes written in `strvec` point between `buffer` and
      `buffer + bufsize - 1`.
*/
int de_unpack_strings(const char *buffer, int64_t bufsize, const char **strvec, int64_t length)
{
    if (buffer == NULL || strvec == NULL)
        return error(DE_NULL);

    const char *p = buffer;
    for (int i = 0; i < length; ++i, ++p)
    {
        if (p - buffer >= bufsize)
        {

            /* didn't find enough strings in buffer -- zero the remaining
            pointers in strvec and return error */
            while (i < length)
                strvec[i++] = NULL;
            return error(DE_ARG);
        }
        strvec[i] = p;
        while (*p != '\0')
            ++p;
    }
    return DE_SUCCESS;
}

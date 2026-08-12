
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"
#include "misc.h"
#include "error.h"

DE_API const char *de_version(void)
{
    return DE_VERSION;
}

DE_API int de_max_axes(void)
{
    return DE_MAX_AXES;
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
DE_API int de_pack_strings(const char **strvec, int64_t length, char *buffer, int64_t *bufsize)
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
DE_API int de_unpack_strings(const char *buffer, int64_t bufsize, const char **strvec, int64_t length)
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

/* functions for extracting values from void pointers */
DE_API double get_double_from_voidptr(const void *p)
{
    const double *pd = (const double *)p;
    return *pd;
}

DE_API int64_t get_int64_from_voidptr(const void *p)
{
    const int64_t *pi = (const int64_t *)p;
    return *pi;
}

DE_API uint64_t get_uint64_from_voidptr(const void *p)
{
    const uint64_t *pu = (const uint64_t *)p;
    return *pu;
}

/* matlab will handle the string pointer */

DE_API const char *get_string_from_voidptr(const void *p)
{
    const char *ps = (const char *)p;
    return ps;
}

DE_API double get_complex_real_from_voidptr(const void *p)
{
    const double *pc = (const double *)p;
    return pc[0]; // Real part is first element
}

DE_API double get_complex_imag_from_voidptr(const void *p)
{
    const double *pc = (const double *)p;
    return pc[1]; // Imaginary part is second element
}

DE_API double get_double_from_voidptr_offset(const void *p, size_t byte_offset)
{
    const char *base = (const char *)p;
    const double *pd = (const double *)(base + byte_offset);
    return *pd;
}

DE_API int64_t get_int64_from_voidptr_offset(const void *p, size_t byte_offset)
{
    const char *base = (const char *)p;
    const int64_t *pi = (const int64_t *)(base + byte_offset);
    return *pi;
}

DE_API uint64_t get_uint64_from_voidptr_offset(const void *p, size_t byte_offset)
{
    const char *base = (const char *)p;
    const uint64_t *pu = (const uint64_t *)(base + byte_offset);
    return *pu;
}

DE_API int32_t get_int32_from_voidptr(const void *p)
{
    const int32_t *pi = (const int32_t *)p;
    return *pi;
}

DE_API signed char get_char_from_voidptr(const void *p)
{
    const signed char *pc = (const signed char *)p;
    return *pc;
}

/* array extractors using memcpy to copy to pre-allocated output array */
// Copy array of double values from void pointer to pre-allocated output array
DE_API void get_double_array_from_voidptr(const void *p, size_t length, double *output)
{
    if (p == NULL || output == NULL || length == 0)
        return; // Safe handling of invalid inputs
    memcpy(output, p, length * sizeof(double));
}

DE_API void get_int64_array_from_voidptr(const void *p, size_t length, int64_t *output)
{
    if (p == NULL || output == NULL || length == 0)
        return; // Safe handling of invalid inputs
    memcpy(output, p, length * sizeof(int64_t));
}

DE_API void get_uint64_array_from_voidptr(const void *p, size_t length, uint64_t *output)
{
    if (p == NULL || output == NULL || length == 0)
        return; // Safe handling of invalid inputs
    memcpy(output, p, length * sizeof(uint64_t));
}

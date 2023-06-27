
#include "error.h"
#include "file.h"
#include "object.h"
#include "axis.h"
#include "tseries.h"
#include "sql.h"

bool check_tseries_type(type_t type)
{
    return type_vector <= type && type <= type_other_1d;
}

/* create a new 1d-array object in a given parent catalog */
int de_store_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                   type_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value,
                   obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    if (!check_tseries_type(type))
        return error(DE_BAD_TYPE);
    while (true)
    {
        if (check_scalar_type(eltype))
            break;
        if (type == type_range && eltype == type_none)
            break;
        return error(DE_BAD_TYPE);
    }
    obj_id_t _id;
    TRACE_RUN(_new_object(de, pid, class_tseries, type, name, &_id));
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_store_tseries_value(de, _id, eltype, axis_id, nbytes, value));
    return DE_SUCCESS;
}

/* load a 1d-array object by name from a given parent catalog */
int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries)
{
    if (de == NULL || tseries == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, &(tseries->object)));
    if (tseries->object.class != class_tseries)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_tseries_value(de, id, tseries));
    return DE_SUCCESS;
}

/*
    pack a vector of strings into a contiguous memory buffer
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
    * If on entry `*bufsize` is sufficiently large, write the packed
      representation into `buffer` (must not be NULL) and the number of bytes
      actually used in `*buffer`.
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

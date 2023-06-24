#ifndef __TSERIES_H__
#define __TSERIES_H__

#include "file.h"
#include "object.h"
#include "axis.h"

/* ========================================================================= */
/* API */

typedef struct
{
    object_t object;
    type_t eltype;
    axis_t axis;
    int64_t nbytes;
    const void *value; /* we don't manage the memory for the value */
} tseries_t;
typedef tseries_t vector_t;

/* create a new 1d-array object in a given parent catalog */
int de_new_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                   type_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value,
                   obj_id_t *id);

/* load a 1d-array object by name from a given parent catalog */
int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries);

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
int de_pack_strings(const char **strvec, int64_t length, char *buffer, int64_t *bufsize);

/* "unpack" a buffer of strings into a vector of '\0'- terminated strings */
int de_unpack_strings(const char *buffer, int64_t bufsize, const char **strvec, int64_t length);

/* ========================================================================= */
/* internal */

#endif

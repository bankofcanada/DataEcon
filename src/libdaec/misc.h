#ifndef __MISC_H__
#define __MISC_H__

#include <stdbool.h>
#include <stdint.h>

#include "config.h"

/* ========================================================================= */
/* API */

/* return a static string containing the library version in format "x.y.z" */
DE_API const char *de_version(void);

/* returns the current setting for DE_MAX_AXES*/
DE_API int de_max_axes(void);

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
DE_API int de_pack_strings(const char **strvec, int64_t length, char *buffer, int64_t *bufsize);

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
DE_API int de_unpack_strings(const char *buffer, int64_t bufsize, const char **strvec, int64_t length);

/* ******************** matlab pointer extractors **************************** */

DE_API double get_double_from_voidptr(const void* p);
DE_API int64_t get_int64_from_voidptr(const void* p);
DE_API uint64_t get_uint64_from_voidptr(const void* p);
DE_API const char* get_string_from_voidptr(const void* p);
DE_API double get_complex_real_from_voidptr(const void* p);
DE_API double get_complex_imag_from_voidptr(const void* p);
/* Array/Vector access functions with byte offset */
DE_API double get_double_from_voidptr_offset(const void* p, size_t byte_offset);
DE_API int64_t get_int64_from_voidptr_offset(const void* p, size_t byte_offset);
DE_API uint64_t get_uint64_from_voidptr_offset(const void* p, size_t byte_offset);
/* Vectorized array extraction functions for performance optimization */
/* Copy array of values from void pointer to pre-allocated output array */
DE_API void get_double_array_from_voidptr(const void* p, size_t length, double* output);
DE_API void get_int64_array_from_voidptr(const void* p, size_t length, int64_t* output);
DE_API void get_uint64_array_from_voidptr(const void* p, size_t length, uint64_t* output);
DE_API int32_t get_int32_from_voidptr(const void* p);
DE_API signed char get_char_from_voidptr(const void* p);

/* ========================================================================= */
/* internal */

#endif

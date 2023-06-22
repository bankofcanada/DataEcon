#ifndef __DE_H__
#define __DE_H__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ***************************** error ************************************* */

    /* Return the result code of the most recent error. If msg != NULL, fill msg with
    the corresponding error message and clear the error. */
    int de_error(char *msg, size_t len);

    /* same, but message contains information about the source of the error. */
    int de_error_source(char *msg, size_t len);

    /* reset error tracking */
    int de_clear_error();

    /* positive error codes come from sqlite: https://sqlite.org/rescode.html */
    enum
    {
        DE_SUCCESS = 0,       /* no error */
        DE_ERR_ALLOC = -1000, /* memory allocation error */
        DE_BAD_AXIS_TYPE,     /* invalid axis type code */
        DE_BAD_CLASS,         /* class of object does not match */
        DE_BAD_TYPE,          /* type of object is not valid for its class */
        DE_BAD_NAME,          /* invalid object name */
        DE_SHORT_BUF,         /* provided buffer is too short */
        DE_OBJ_DNE,           /* object does not exist */
        DE_AXIS_DNE,          /* axis does not exist */
        DE_ARG,               /* invalid combination of arguments */
        DE_NO_OBJ,            /* no more objects in search list */
        DE_EXISTS,            /* object already exists */
        DE_BAD_OBJ,           /* bad object */
        DE_NULL,              /* call with NULL pointer */
        DE_DEL_ROOT,          /* cannot delete the root catalog */
        DE_MIS_ATTR,          /* missing attribute (name) */
        DE_INTERNAL,          /* internal error */
    };

    /* ***************************** file ************************************* */

    typedef void *de_file;

    /* open daec file */
    int de_open(const char *fname, de_file *de);

    /* close a previously opened daec file */
    int de_close(de_file de);

    /* ***************************** object  ************************************* */

    typedef enum
    {
        class_catalog = 0,
        class_scalar,
        class_vector, /* 1d array */
        class_tseries = class_vector,
        class_matrix, /* 2d array*/
        class_mvtseries = class_matrix,
        class_any = -1
    } class_t;

    typedef enum
    {
        type_none = 0,    /* for object that don't have a type, e.g. class_catalog */
        type_integer = 1, /* stored in scalars */
        type_signed = type_integer,
        type_unsigned,
        type_date,
        type_float,
        type_complex,
        type_string,
        type_other_scalar,
        type_vector = 10, /* stored in arrays1d */
        type_range,
        type_tseries,
        type_other_1d,
        type_matrix = 20, /* stored in arrays2d */
        type_mvtseries,
        type_other_2d,
        type_any = -1
    } type_t;

    typedef enum
    {
        freq_none = 0,
        freq_unit = 1,
        freq_daily = 4,
        freq_bdaily = 5,
        freq_monthly = 8,
        freq_weekly = 16,
        freq_weekly_sun = freq_weekly,
        freq_weekly_mon,
        freq_weekly_tue,
        freq_weekly_wed,
        freq_weekly_thu,
        freq_weekly_fri,
        freq_weekly_sat,
        freq_quarterly = 32,
        freq_quarterly_jan = freq_quarterly + 1,
        freq_quarterly_feb,
        freq_quarterly_mar,
        freq_quarterly_apr = freq_quarterly + 1,
        freq_quarterly_may,
        freq_quarterly_jun,
        freq_quarterly_jul = freq_quarterly + 1,
        freq_quarterly_aug,
        freq_quarterly_sep,
        freq_quarterly_oct = freq_quarterly + 1,
        freq_quarterly_nov,
        freq_quarterly_dec,
        freq_halfyearly = 64,
        freq_halfyearly_jan = freq_halfyearly + 1,
        freq_halfyearly_feb,
        freq_halfyearly_mar,
        freq_halfyearly_apr,
        freq_halfyearly_may,
        freq_halfyearly_jun,
        freq_halfyearly_jul = freq_halfyearly + 1,
        freq_halfyearly_aug,
        freq_halfyearly_sep,
        freq_halfyearly_oct,
        freq_halfyearly_nov,
        freq_halfyearly_dec,
        freq_yearly = 128,
        freq_yearly_jan = freq_yearly + 1,
        freq_yearly_feb,
        freq_yearly_mar,
        freq_yearly_apr,
        freq_yearly_may,
        freq_yearly_jun,
        freq_yearly_jul,
        freq_yearly_aug,
        freq_yearly_sep,
        freq_yearly_oct,
        freq_yearly_nov,
        freq_yearly_dec,
    } frequency_t;

    typedef int64_t obj_id_t;

    /* an instance of object_t corresponds to a row in the `catalog` table*/
    /* we don't manage the memory for the name */
    typedef struct
    {
        obj_id_t id;
        obj_id_t pid;
        class_t class;
        type_t type;
        const char *name;
    } object_t;

    /* find object id from parent and name */
    int de_find_object(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

    /* load object from id*/
    int de_load_object(de_file de, obj_id_t id, object_t *object);

    /* delete object given id*/
    int de_delete_object(de_file de, obj_id_t id);

    /* set attribute by name */
    int de_set_attribute(de_file de, obj_id_t id, const char *name, const char *value);

    /* get attribute by name */
    int de_get_attribute(de_file de, obj_id_t id, const char *name, const char **value);

    /* get all attributes in a single string with format '"name"="value"' separated by `delim` */
    int de_get_all_attributes(de_file de, obj_id_t id, const char *delim,
                              int64_t *nattr, const char **names, const char **values);

    /* get the full path of an object from its id */
    int de_get_object_info(de_file, obj_id_t id,
                           const char **fullpath, int64_t *depth, int64_t *created);

    /* get the id of an object from its fullpath */
    int de_find_fullpath(de_file de, const char *fullpath, obj_id_t *id);

    /* ***************************** catalog ************************************* */

    typedef struct
    {
        object_t object;
    } catalog_t;

    /* create new catalog. return error if catalog already exists */
    int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

    /* load a catalog object from its id */
    int de_load_catalog(de_file de, obj_id_t id, catalog_t *catalog);

    /* ***************************** scalar ************************************* */

    typedef struct
    {
        object_t object;
        frequency_t frequency;
        int64_t nbytes;
        const void *value; /* we don't manage the memory for the value */
    } scalar_t;

    /* create a new scalar object in a given parent catalog */
    int de_new_scalar(de_file de, obj_id_t pid, const char *name, type_t type,
                      frequency_t freq, int64_t nbytes, const void *value,
                      obj_id_t *id);

    /* load a scalar object by name from a given parent catalog */
    int de_load_scalar(de_file de, obj_id_t id, scalar_t *scalar);

    /* ***************************** axis ************************************* */

    typedef int64_t axis_id_t;

    typedef enum
    {
        axis_plain = 0, /* a simple numerical range, e.g. 1:5 */
        axis_range,     /* a range of dates, e.g. 2020Q1:2025Q4 */
        axis_names,     /* an ordered list of variable names, e.g. ("color", "length", "shape") */
    } axis_type_t;

    typedef struct
    {
        axis_id_t id;
        axis_type_t type;
        int64_t length;
        frequency_t frequency;
        /* at most one of these is valid, depending on type */
        int64_t first;
        const char *names;
    } axis_t;

    /* create or find an axis of the given type */
    int de_axis_plain(de_file de, int64_t length, axis_id_t *id);
    int de_axis_range(de_file de, int64_t length, frequency_t frequency, int64_t first, axis_id_t *id);
    int de_axis_names(de_file de, int64_t length, const char *names, axis_id_t *id);

    /* load an axis from its id */
    int de_load_axis(de_file de, axis_id_t id, axis_t *axis);

    /* ***************************** tseries ************************************* */

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

    /* ***************************** mvtseries ************************************* */

    /* ***************************** search ************************************* */

    typedef void *de_search;

    int de_list_catalog(de_file de, obj_id_t pid, de_search *search);
    int de_search_catalog(de_file de, obj_id_t pid, const char *wc,
                          type_t type, class_t cls, de_search *search);

    int de_next_object(de_search search, object_t *object);
    int de_finalize_search(de_search search);

#ifdef __cplusplus
}
#endif

#endif
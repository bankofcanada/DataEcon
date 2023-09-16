#ifndef __DE_H__
#define __DE_H__

#include <stdlib.h>
#include <stdint.h>

/* ***************************** config ************************************** */
#define DE_VERSION "0.2.5"
#define DE_VERNUM 0x0250
#define DE_VER_MAJOR 0
#define DE_VER_MINOR 2
#define DE_VER_REVISION 5
#define DE_VER_SUBREVISION 0

#ifdef __cplusplus
extern "C"
{
#endif

    /* return the library version as a string in "x.y.z" format */
    const char *de_version(void);

    /* ***************************** error *************************************** */

    /* Return the result code of the most recent error. If msg != NULL, fill msg with
    the corresponding error message and clear the error. */
    int de_error(char *msg, size_t len);

    /* same, but message contains information about the source of the error. */
    int de_error_source(char *msg, size_t len);

    /* reset error tracking */
    int de_clear_error(void);

    /* positive error codes come from sqlite: https://sqlite.org/rescode.html */
    enum
    {
        DE_SUCCESS = 0,       /* no error */
        DE_ERR_ALLOC = -1000, /* memory allocation error */
        DE_BAD_AXIS_TYPE,     /* invalid axis type code */
        DE_BAD_CLASS,         /* class of object does not match */
        DE_BAD_TYPE,          /* type of object is not valid for its class */
        DE_BAD_NAME,          /* invalid object name */
        DE_BAD_FREQ,          /* bad frequency */
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
        DE_INEXACT,           /* inexact date conversion, e.g. Saturday or Sunday specified as business daily date */
        DE_RANGE,             /* value out of range */
        DE_INTERNAL,          /* internal error */
    };

    /* ***************************** file **************************************** */

    typedef void *de_file;

    /* open daec file */
    int de_open(const char *fname, de_file *de);

    /* open a daec database in memory */
    int de_open_memory(de_file *pde);

    /* close a previously opened daec file */
    int de_close(de_file de);

    /* delete everything in the given daec file */
    int de_truncate(de_file de);

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
    int de_get_object_info(de_file de, obj_id_t id,
                           const char **fullpath, int64_t *depth, int64_t *created);

    /* get the id of an object from its fullpath */
    int de_find_fullpath(de_file de, const char *fullpath, obj_id_t *id);

    /* ***************************** catalog ************************************* */

    /* create new catalog. return error if catalog already exists */
    int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

    /* ***************************** date **************************************** */

    typedef enum
    {
        freq_none = 0,
        freq_unit = 1,
        freq_daily = 4,
        freq_bdaily = 5,
        freq_weekly = 16,
        freq_weekly_sun0 = freq_weekly,
        freq_weekly_mon,
        freq_weekly_tue,
        freq_weekly_wed,
        freq_weekly_thu,
        freq_weekly_fri,
        freq_weekly_sat,
        freq_weekly_sun7,
        freq_weekly_sun = freq_weekly_sun7,
        freq_monthly = 32,
        freq_quarterly = 64,
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
        freq_halfyearly = 128,
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
        freq_yearly = 256,
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

    typedef int64_t date_t;

    int de_pack_year_period_date(frequency_t freq, int32_t year, uint32_t period, date_t *date);
    int de_unpack_year_period_date(frequency_t freq, date_t date, int32_t *year, uint32_t *period);

    int de_pack_calendar_date(frequency_t freq, int32_t year, uint32_t month, uint32_t day, date_t *date);
    int de_unpack_calendar_date(frequency_t freq, date_t date, int32_t *year, uint32_t *month, uint32_t *day);

    /* ***************************** scalar ************************************** */

    typedef struct
    {
        object_t object;
        frequency_t frequency;
        int64_t nbytes;
        const void *value; /* we don't manage the memory for the value */
    } scalar_t;

    /* create a new scalar object in a given parent catalog */
    int de_store_scalar(de_file de, obj_id_t pid, const char *name, type_t type,
                        frequency_t freq, int64_t nbytes, const void *value,
                        obj_id_t *id);

    /* load a scalar object by name from a given parent catalog */
    int de_load_scalar(de_file de, obj_id_t id, scalar_t *scalar);

    /* ***************************** axis **************************************** */

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
    int de_store_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                         type_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value,
                         obj_id_t *id);

    /* load a 1d-array object by name from a given parent catalog */
    int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries);

    /* ***************************** mvtseries *********************************** */

    typedef struct
    {
        object_t object;
        type_t eltype;
        axis_t axis1;
        axis_t axis2;
        int64_t nbytes;
        const void *value; /* we don't manage the memory for the value */
    } mvtseries_t;
    typedef mvtseries_t matrix_t;

    /* create a new 1d-array object in a given parent catalog */
    int de_store_mvtseries(de_file de, obj_id_t pid, const char *name, type_t type,
                           type_t eltype, axis_id_t axis1_id, axis_id_t axis2_id,
                           int64_t nbytes, const void *value,
                           obj_id_t *id);

    /* load a 1d-array object by name from a given parent catalog */
    int de_load_mvtseries(de_file de, obj_id_t id, mvtseries_t *mvtseries);

    /* ***************************** misc **************************************** */

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
    int de_pack_strings(const char **strvec, int64_t length, char *buffer, int64_t *bufsize);

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
    int de_unpack_strings(const char *buffer, int64_t bufsize, const char **strvec, int64_t length);

    /* ***************************** search ************************************** */

    typedef void *de_search;

    /* start a search that yields all objects in the given catalog id */
    int de_list_catalog(de_file de, obj_id_t pid, de_search *search);

    /* start a search that yields objects in the given catalog, that
    additionally match the given criteria for wildcard, type and class. To skip
    individual filters, set wc to NULL, type to type_any and class to class_any.
    */
    int de_search_catalog(de_file de, obj_id_t pid, const char *wc,
                          type_t type, class_t cls, de_search *search);

    /* Return DE_SUCCESS and load the next object in a search. Return DE_NO_OBJ
    when search is done. */
    int de_next_object(de_search search, object_t *object);

    /* Release resources allocated for the given search. */
    int de_finalize_search(de_search search);

#ifdef __cplusplus
}
#endif

#endif

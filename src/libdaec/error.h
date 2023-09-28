#ifndef __ERROR_H__
#define __ERROR_H__

#include <sqlite3.h>
#include <stddef.h>
#include <stdint.h>

/* ========================================================================= */
/* API */

/* Return the result code of the most recent error. If msg != NULL, fill msg with
the corresponding error message and clear the error. */
int de_error(char *restrict msg, size_t len);

/* same, but message contains information about the source of the error. */
int de_error_source(char *restrict msg, size_t len);

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
    DE_BAD_ELTYPE_DATE,   /* element type date should be specified with its frequency code */
    DE_BAD_ELTYPE_NONE,   /* element type set to NONE for object type other than range */
    DE_BAD_NAME,          /* invalid object name */
    DE_BAD_FREQ,          /* bad frequency */
    DE_SHORT_BUF,         /* provided buffer is too short */
    DE_OBJ_DNE ,           /* object does not exist */
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

/* ========================================================================= */
/* internal */

/* length of buffer to store name of file where error occurred. */
#define _MAXFILE 48

struct error_s;
typedef struct error_s error_t;

/* error without any arguments */
#define error(code) set_error((code), __func__, __FILE__, __LINE__)
int set_error(int errcode, const char *func, const char *file, int line);

/* error with one argument */
#define error1(code, arg) set_error1((code), (arg), __func__, __FILE__, __LINE__)
int set_error1(int errcode, const char *arg, const char *func, const char *file, int line);

/* error from SQLite database */
#define db_error(de) set_db_error((de)->db, __func__, __FILE__, __LINE__)
int set_db_error(sqlite3 *db, const char *func, const char *file, int line);

/* error from SQLite result code */
#define rc_error(rc) set_rc_error((rc), __func__, __FILE__, __LINE__)
int set_rc_error(int rc, const char *func, const char *file, int line);

#define trace_error() set_trace_error(__func__, __FILE__, __LINE__)
int set_trace_error(const char *func, const char *file, int line);

#define TRACE_RUN(what)                                       \
    if (DE_SUCCESS != (what))                                 \
    {                                                         \
        return set_trace_error(__func__, __FILE__, __LINE__); \
    }

#endif

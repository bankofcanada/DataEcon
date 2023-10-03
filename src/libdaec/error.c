
#include "error.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

/* length of static buffer where the last sqlite3 error message is stored. */
#define _MAX_MSG 1024

/* static buffer holding the last error message from sqlite3 */
static char last_s3_msg[_MAX_MSG] = "\0";

#define _MAX_TRACE (4096 - sizeof(int) - 2 * sizeof(const char *))

struct error_s
{
    int code;
    const char *s3_msg;
    char *arg;
    char source_trace[_MAX_TRACE];
};

static error_t last_error = {
    .code = 0,
    .s3_msg = NULL,
    .arg = NULL,
    .source_trace = "\0",
};

int de_clear_error(void)
{
    /* we don't free memory for s3_msg: it is managed by sqlite3 https://www.sqlite.org/c3ref/errcode.html */
    if (last_error.arg)
        free(last_error.arg);
    memset(&last_error, 0, sizeof(last_error));
    return DE_SUCCESS;
}

void _push_trace(const char *func, const char *file, int line)
{
    char *trace = last_error.source_trace;
    int len = strlen(trace);
    // const char *basename = strrchr(file, '/');
    // if (basename == NULL)
    //     basename = strrchr(file, '\\');
    // basename = (basename == NULL) ? file : basename + 1;
    // snprintf(trace + len, _MAX_TRACE - len, "\n%6s: %s (%s:%d)", len == 0 ? "in" : "", func, basename, line);
    snprintf(trace + len, _MAX_TRACE - len, "\n%6s: %s (%s:%d)", len == 0 ? "in" : "", func, file, line);
}

/* error with no arguments */
int set_error(int code, const char *func, const char *file, int line)
{
    de_clear_error();
    last_error.code = code;
    _push_trace(func, file, line);
    return code;
}

/* error with one argument */
int set_error1(int code, const char *arg, const char *func, const char *file, int line)
{
    set_error(code, func, file, line);
    int len = strlen(arg);
    if (len > 0)
    {
        last_error.arg = malloc(len + 1);
        strncpy(last_error.arg, arg, len + 1);
    }
    return code;
}

/* error from sqlite3 database */
int set_db_error(sqlite3 *db, const char *func, const char *file, int line)
{
    int code = set_error(sqlite3_errcode(db), func, file, line);
    if (db)
    {
        /* memory returned by sqlite3_errmsg is managed by sqlite3: https://www.sqlite.org/c3ref/errcode.html */
        /* we need a copy because sqlite3 might free or overwrite */
        last_error.s3_msg = strncpy(last_s3_msg, sqlite3_errmsg(db), _MAX_MSG - 1);
    }
    return code;
}

/* error from sqlite3 result code */
int set_rc_error(int rc, const char *func, const char *file, int line)
{
    set_error(rc, func, file, line);
    /* memory returned by sqlite3_errstr is managed by sqlite3: https://www.sqlite.org/c3ref/errcode.html */
    /* we need a copy because sqlite3 might free or overwrite */
    last_error.s3_msg = strncpy(last_s3_msg, sqlite3_errstr(rc), _MAX_MSG - 1);
    return rc;
}

/* continue propagating the same error */
int set_trace_error(const char *func, const char *file, int line)
{
    _push_trace(func, file, line);
    return last_error.code;
}

/* return last error code, fill in last error message in user-supplied buffer and clear error */
int de_error(char *restrict msg, size_t len)
{
    const static char fmt[] = "DE(%d): %s";
    const static char fmt1[] = "DE(%d): %s -- %s";
    const static char fmt_s3[] = "DE(%d) SQLite3: %s";

    int code = last_error.code;
    if (msg != NULL)
    {
        /* fill in the error message in the provided buffer */
        switch (code)
        {
        case DE_SUCCESS:
            snprintf(msg, len, fmt, code, "no error");
            break;
        case DE_ERR_ALLOC:
            snprintf(msg, len, fmt, code, "memory allocation error");
            break;
        case DE_BAD_AXIS_TYPE:
            snprintf(msg, len, fmt, code, "invalid axis type code");
            break;
        case DE_BAD_CLASS:
            snprintf(msg, len, fmt, code, "class of object does not match");
            break;
        case DE_BAD_TYPE:
            snprintf(msg, len, fmt, code, "type of object is not valid for its class");
            break;
        case DE_BAD_ELTYPE:
            snprintf(msg, len, fmt, code, "element type is not scalar");
            break;
        case DE_BAD_ELTYPE_NONE:
            snprintf(msg, len, fmt, code, "element type is type_none(0) for an object type other than range");
            break;
        case DE_BAD_ELTYPE_DATE:
            snprintf(msg, len, fmt, code, "element type is date must have element frequency other than freq_none (0)");
            break;
        case DE_BAD_NAME:
            snprintf(msg, len, fmt1, code, "invalid object name", last_error.arg);
            break;
        case DE_BAD_FREQ:
            snprintf(msg, len, fmt, code, "bad frequency");
            break;
        case DE_SHORT_BUF:
            snprintf(msg, len, fmt, code, "provided buffer is too short");
            break;
        case DE_OBJ_DNE:
            snprintf(msg, len, fmt1, code, "object does not exist", last_error.arg);
            break;
        case DE_AXIS_DNE:
            snprintf(msg, len, fmt, code, "axis does not exist");
            break;
        case DE_ARG:
            snprintf(msg, len, fmt, code, "invalid argument or combination of arguments");
            break;
        case DE_NO_OBJ:
            snprintf(msg, len, fmt, code, "no more objects");
            break;
        case DE_EXISTS:
            snprintf(msg, len, fmt1, code, "object already exists", last_error.arg);
            break;
        case DE_BAD_OBJ:
            snprintf(msg, len, fmt, code, "inconsistent data - possible database corruption");
            break;
        case DE_NULL:
            snprintf(msg, len, fmt, code, "call with NULL pointer");
            break;
        case DE_DEL_ROOT:
            snprintf(msg, len, fmt, code, "must not delete the root catalog");
            break;
        case DE_MIS_ATTR:
            snprintf(msg, len, fmt1, code, "missing attribute", last_error.arg);
            break;
        case DE_INEXACT:
            snprintf(msg, len, fmt, code, "inexact date conversion, e.g. Saturday or Sunday specified as business daily date");
            break;
        case DE_RANGE:
            snprintf(msg, len, fmt, code, "value out of range");
            break;
        case DE_INTERNAL:
            snprintf(msg, len, fmt1, code, "internal error", last_error.arg);
            break;
        default:
            if (code > 0)
            {
                /* the error came from sqlite */
                snprintf(msg, len, fmt_s3, code, last_error.s3_msg);
            }
            else
            {
                /* don't know where this came from */
                snprintf(msg, len, fmt, code, "invalid error code");
            }
            break;
        }
        de_clear_error();
    }
    return code;
}

/* (for use by developers) same as de_error(), but also add information about where in the code the error happened */
int de_error_source(char *restrict msg, size_t total_len)
{
    error_t le;
    memcpy(&le, &last_error, sizeof le);
    int code = de_error(msg, total_len);
    if (code != DE_SUCCESS && msg != NULL)
    {
        int len = strlen(msg);
        if (len < total_len)
            snprintf(msg + len, total_len - len, "%s", le.source_trace);
    }
    return code;
}


#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "search.h"
#include "sql.h"

static char *_push_string(char *p, const char *str)
{
    char nul = '\0';
    while (*str != nul)
    {
        *p = *str;
        ++str;
        ++p;
    }
    *p = nul;
    return p;
}

int _prepare_search(de_file de, int64_t pid, const char *wc, type_t type, class_t class, search_t *search)
{
    enum
    {
        _BUF_SIZE = 512,
    };

    char buf[_BUF_SIZE];
    char *p = _push_string(buf, "SELECT `id`, `pid`, `class`, `type`, `name` FROM `objects` WHERE TRUE");

    assert(((long int)(p - buf) < _BUF_SIZE));

    if (pid >= 0)
    {
        p = _push_string(p, " AND `pid` = ?");
        assert(((long int)(p - buf) < _BUF_SIZE));
    }
    if (wc != NULL)
    {
        p = _push_string(p, " AND `name` GLOB ?");
        assert(((long int)(p - buf) < _BUF_SIZE));
    }
    if (type != type_any)
    {
        p = _push_string(p, " AND `type` = ?");
        assert(((long int)(p - buf) < _BUF_SIZE));
    }
    if (class != class_any)
    {
        p = _push_string(p, " AND `class` = ?");
        assert(((long int)(p - buf) < _BUF_SIZE));
    }

    /* add ordering - actually do we need that here? no. */
    // p = _push_string(p, " ORDER BY `pid`, `id`;");
    // assert(((long int)(p - buf) < _BUF_SIZE));

    /* fill the rest of buf with 0 (not really needed, just being pedantic)*/
    memset(p, 0, _BUF_SIZE - (p - buf));

    int rc;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(de->db, buf, p - buf + 1, &stmt, NULL);
    if (rc != SQLITE_OK)
        return rc_error(rc);

#define BIND_PARAM(call)            \
    {                               \
        rc = call;                  \
        if (rc != SQLITE_OK)        \
        {                           \
            sqlite3_finalize(stmt); \
            return rc_error(rc);    \
        }                           \
    }
    int ipar = 0;
    /* must be in the same order as above */
    if (pid >= 0)
        BIND_PARAM(sqlite3_bind_int64(stmt, ++ipar, pid));
    if (wc != NULL)
        BIND_PARAM(sqlite3_bind_text(stmt, ++ipar, wc, -1, SQLITE_TRANSIENT));
    if (type != type_any)
        BIND_PARAM(sqlite3_bind_int(stmt, ++ipar, type));
    if (class != class_any)
        BIND_PARAM(sqlite3_bind_int(stmt, ++ipar, class));
#undef BIND_PARAM

    search->stmt = stmt;
    return DE_SUCCESS;
}

int de_list_catalog(de_file de, obj_id_t pid, de_search *search)
{
    if (de == NULL || search == NULL)
        return error(DE_NULL);
    *search = calloc(1, sizeof(search_t));
    if (*search == NULL)
        return error(DE_ERR_ALLOC);
    TRACE_RUN(_prepare_search(de, pid, NULL, type_any, class_any, *search));
    return DE_SUCCESS;
}

int de_search_catalog(de_file de, obj_id_t pid, const char *wc, type_t type, class_t class, de_search *search)
{
    if (de == NULL || search == NULL)
        return error(DE_NULL);
    *search = calloc(1, sizeof(search_t));
    if (*search == NULL)
        return error(DE_ERR_ALLOC);
    TRACE_RUN(_prepare_search(de, pid, wc, type, class, *search));
    return DE_SUCCESS;
}

int de_next_object(de_search search, object_t *object)
{
    if (search == NULL || object == NULL)
        return error(DE_NULL);

    int rc = sqlite3_step(search->stmt);
    switch (rc)
    {
    case SQLITE_ROW:
        _fill_object(search->stmt, object);
        return DE_SUCCESS;
    case SQLITE_DONE:
        sqlite3_finalize(search->stmt);
        search->stmt = NULL;
        return error(DE_NO_OBJ);
    default:
        return rc_error(rc);
    }
}

int de_finalize_search(de_search search)
{
    if (search == NULL)
        return DE_SUCCESS;
    /* https://www.sqlite.org/c3ref/finalize.html */
    /* sqlite3_finalize(NULL) is a harmless no-op. Also, returns error code if the last statement operation failed*/
    int rc = sqlite3_finalize(search->stmt);
    if (rc != SQLITE_OK)
        return rc_error(rc);
    search->stmt = NULL;
    free(search);
    /* we call clear_error because last call to de_next_object may have returned DE_NO_OBJ */
    de_clear_error();
    return DE_SUCCESS;
}

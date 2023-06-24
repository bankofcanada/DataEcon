
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "sql.h"

/* check if the given string is a valid object name */
bool _check_name(const char *name)
{
    /* check that name is not empty */
    if (*name == '\0')
    {
        error1(DE_BAD_NAME, "empty");
        return false;
    }
    bool blank = true;
    for (const char *p = name; *p != '\0'; ++p)
    {
        /* check that the name does not contain a slash */
        if (*p == '/')
        {
            error1(DE_BAD_NAME, "contains '/'");
            return false;
        }
        /* check that the name contains at least one non-space character */
        if (blank && !isspace(*p))
            blank = false;
    }
    if (blank)
    {
        error1(DE_BAD_NAME, "blank");
        return false;
    }
    return true;
}

int _new_object(de_file de, obj_id_t pid, class_t class, type_t type, const char *name, obj_id_t *id)
{
    if (!_check_name(name))
        return trace_error();
    int rc;
    rc = sql_find_object(de, pid, name, NULL);
    if (rc == DE_SUCCESS)
        return error1(DE_EXISTS, name);
    if (rc != DE_OBJ_DNE)
        return trace_error();
    de_clear_error();
    TRACE_RUN(de_begin_transaction(de));
    TRACE_RUN(sql_new_object(de, pid, class, type, name));
    obj_id_t _id = sqlite3_last_insert_rowid(de->db);
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_new_object_info(de, _id));
    return DE_SUCCESS;
}

int de_find_object(de_file db, obj_id_t pid, const char *name, obj_id_t *id)
{
    if (db == NULL || name == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_find_object(db, pid, name, id));
    return DE_SUCCESS;
}

int de_load_object(de_file de, obj_id_t id, object_t *object)
{
    if (de == NULL || object == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, object));
    return DE_SUCCESS;
}

int de_delete_object(de_file de, obj_id_t id)
{
    if (de == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_delete_object(de, id));
    return DE_SUCCESS;
}

int de_set_attribute(de_file de, obj_id_t id, const char *name, const char *value)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_set_attribute(de, id, name, value));
    return DE_SUCCESS;
}

int de_get_attribute(de_file de, obj_id_t id, const char *name, const char **value)
{
    if (de == NULL || name == NULL || value == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_get_attribute(de, id, name, value));
    return DE_SUCCESS;
}

int de_get_all_attributes(de_file de, obj_id_t id, const char *delim,
                          int64_t *nattr, const char **names, const char **values)
{
    if (de == NULL || delim == NULL || nattr == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_get_all_attributes(de, id, delim, nattr, names, values));
    return DE_SUCCESS;
}

int de_get_object_info(de_file de, obj_id_t id, const char **fullpath, int64_t *depth, int64_t *created)
{
    if (de == NULL || (fullpath == NULL && depth == NULL && created == NULL))
        return error(DE_NULL);
    TRACE_RUN(sql_get_object_info(de, id, fullpath, depth, created));
    if (id == 0)
        *fullpath = "/";
    return DE_SUCCESS;
}

int de_find_fullpath(de_file de, const char *fullpath, obj_id_t *id)
{
    if (de == NULL || fullpath == NULL || id == NULL)
        return error(DE_NULL);
    if (strcmp(fullpath, "/") == 0)
    {
        *id = 0;
    }
    else
    {
        TRACE_RUN(sql_find_fullpath(de, fullpath, id));
    }
    return DE_SUCCESS;
}

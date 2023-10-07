
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "scalar.h"
#include "axis.h"
#include "tseries.h"
#include "mvtseries.h"
#include "sql.h"
#include "misc.h"

#ifdef CHECK_SQLITE
#undef CHECK_SQLITE
#endif
#define CHECK_SQLITE(s3call)              \
    {                                     \
        if (SQLITE_OK != (rc = (s3call))) \
            return rc_error(rc);          \
    }

int sql_find_object(de_file de, obj_id_t pid, const char *name, obj_id_t *id)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_find_object);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, pid));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        if (id)
            *id = sqlite3_column_int64(stmt, 0);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_OBJ_DNE, _pidnm2str(pid, name));
    default:
        return rc_error(rc);
    }
}

void _fill_object(sqlite3_stmt *stmt, object_t *object)
{
    object->id = sqlite3_column_int64(stmt, 0);
    object->pid = sqlite3_column_int64(stmt, 1);
    object->obj_class = sqlite3_column_int(stmt, 2);
    object->obj_type = sqlite3_column_int(stmt, 3);
    object->name = (const char *)sqlite3_column_text(stmt, 4);
}

int sql_load_object(de_file de, obj_id_t id, object_t *object)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_load_object);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        _fill_object(stmt, object);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_OBJ_DNE, _id2str(id));
    default:
        return rc_error(rc);
    }
    return DE_SUCCESS;
}

int sql_new_object(de_file de, obj_id_t pid, class_t class, type_t type, const char *name)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_new_object);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, pid));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 2, class));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 3, type));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 4, name, -1, SQLITE_TRANSIENT));
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

int sql_new_object_info(de_file de, obj_id_t id)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_new_object_info);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

int sql_delete_object(de_file de, obj_id_t id)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_delete_object);
    if (stmt == NULL)
        return trace_error();
    if (id == 0)
        return error(DE_DEL_ROOT);
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

int sql_set_attribute(de_file de, int64_t id, const char *name, const char *value)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_set_attribute);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT));
    if (value == NULL)
    {
        CHECK_SQLITE(sqlite3_bind_null(stmt, 3));
    }
    else
    {
        CHECK_SQLITE(sqlite3_bind_text(stmt, 3, value, -1, SQLITE_TRANSIENT));
    }
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

int sql_get_attribute(de_file de, int64_t id, const char *name, const char **value)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_get_attribute);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        if (value != NULL)
            *value = (const char *)sqlite3_column_text(stmt, 0);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_MIS_ATTR, name);
    default:
        return rc_error(rc);
    }
}

int sql_get_all_attributes(de_file de, obj_id_t id, const char *delim,
                           int64_t *nattr, const char **names, const char **values)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_get_all_attributes);
    if (stmt == NULL)
        return trace_error();
    int rc;
    int64_t count;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 1, delim, -1, SQLITE_TRANSIENT));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 2, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        count = sqlite3_column_int64(stmt, 0);
        if (nattr != NULL)
            *nattr = count;
        if (names != NULL)
            *names = count == 0 ? NULL : (const char *)sqlite3_column_text(stmt, 1);
        if (values != NULL)
            *values = count == 0 ? NULL : (const char *)sqlite3_column_text(stmt, 2);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_OBJ_DNE, _id2str(id));
    default:
        return rc_error(rc);
    }
}

int sql_get_object_info(de_file de, obj_id_t id, const char **fullpath, int64_t *depth, int64_t *created)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_get_object_info);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        if (fullpath)
            *fullpath = (const char *)sqlite3_column_text(stmt, 0);
        if (depth)
            *depth = sqlite3_column_int64(stmt, 1);
        if (created)
            *created = sqlite3_column_int64(stmt, 2);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_OBJ_DNE, _id2str(id));
    default:
        return rc_error(rc);
    }
}

int sql_find_fullpath(de_file de, const char *fullpath, obj_id_t *id)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_find_fullpath);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_text(stmt, 1, fullpath, -1, SQLITE_TRANSIENT));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        *id = sqlite3_column_int64(stmt, 0);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error1(DE_OBJ_DNE, fullpath);
    default:
        return rc_error(rc);
    }
}

int sql_store_scalar_value(de_file de, obj_id_t id, frequency_t frequency, int64_t nbytes, const void *value)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_store_scalar);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 2, frequency));
    if (value != NULL && nbytes > 0)
    {
        CHECK_SQLITE(sqlite3_bind_blob(stmt, 3, value, nbytes, SQLITE_TRANSIENT));
    }
    else
    {
        CHECK_SQLITE(sqlite3_bind_null(stmt, 3));
    }
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

void _fill_scalar(sqlite3_stmt *stmt, scalar_t *scalar)
{
    obj_id_t id = sqlite3_column_int64(stmt, 0);
    if (id != scalar->object.id)
        error(DE_BAD_OBJ);
    scalar->frequency = sqlite3_column_int(stmt, 1);
    scalar->nbytes = sqlite3_column_bytes(stmt, 2);
    scalar->value = sqlite3_column_blob(stmt, 2);
}

int sql_load_scalar_value(de_file de, obj_id_t id, scalar_t *scalar)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_load_scalar);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        _fill_scalar(stmt, scalar);
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error(DE_BAD_OBJ);
    default:
        /* anything else is an sqlite3 error */
        return rc_error(rc);
    }
}

/******************************************************************/
/* axis */

int sql_load_axis(de_file de, axis_id_t id, axis_t *axis)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_load_axis);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        axis->id = id;
        axis->ax_type = sqlite3_column_int(stmt, 1);
        axis->length = sqlite3_column_int64(stmt, 2);
        axis->frequency = sqlite3_column_int(stmt, 3);
        switch (axis->ax_type)
        {
        case axis_plain:
            axis->first = 0;
            axis->names = NULL;
            break;
        case axis_range:
            axis->first = sqlite3_column_int64(stmt, 4);
            axis->names = NULL;
            break;
        case axis_names:
            axis->first = 0;
            axis->names = (const char *)sqlite3_column_text(stmt, 4);
            break;
        default:
            return error(DE_BAD_AXIS_TYPE);
        }
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error(DE_AXIS_DNE);
    default:
        return rc_error(rc);
    }
}

int sql_find_axis(de_file de, axis_t *axis)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_find_axis);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 1, axis->ax_type));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 2, axis->length));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 3, axis->frequency));
    while (1)
    {
        switch ((rc = sqlite3_step(stmt)))
        {
        case SQLITE_ROW:
            switch (axis->ax_type)
            {
            case axis_plain:
                axis->id = sqlite3_column_int64(stmt, 0);
                return DE_SUCCESS;
            case axis_range:
                if (axis->first == sqlite3_column_int64(stmt, 1))
                {
                    axis->id = sqlite3_column_int64(stmt, 0);
                    return DE_SUCCESS;
                }
                break;
            case axis_names:
                if (strcmp(axis->names, (const char *)sqlite3_column_text(stmt, 1)) == 0)
                {
                    axis->id = sqlite3_column_int64(stmt, 0);
                    return DE_SUCCESS;
                }
                break;
            default:
                return error(DE_BAD_AXIS_TYPE);
            }
            break;
        case SQLITE_DONE:
            return error(DE_AXIS_DNE);
        default:
            return rc_error(rc);
        }
    }
}

int sql_new_axis(de_file de, axis_t *axis)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_new_axis);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 1, axis->ax_type));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 2, axis->length));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 3, axis->frequency));
    switch (axis->ax_type)
    {
    case axis_plain:
        CHECK_SQLITE(sqlite3_bind_null(stmt, 4));
        break;
    case axis_range:
        CHECK_SQLITE(sqlite3_bind_int64(stmt, 4, axis->first));
        break;
    case axis_names:
        CHECK_SQLITE(sqlite3_bind_text(stmt, 4, axis->names, -1, SQLITE_TRANSIENT));
        break;
    default:
        return error(DE_BAD_AXIS_TYPE);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
    {
        axis->id = sqlite3_last_insert_rowid(de->db);
        return DE_SUCCESS;
    }
    return rc_error(rc);
}

/**************************************************************/
/* tseries */

int sql_store_tseries_value(de_file de, obj_id_t id,
                            type_t eltype, frequency_t elfreq,
                            axis_id_t axis_id, int64_t nbytes,
                            const void *value)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_store_tseries);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 2, eltype));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 3, elfreq));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 4, axis_id));
    if (value != NULL && nbytes > 0)
    {
        CHECK_SQLITE(sqlite3_bind_blob(stmt, 5, value, nbytes, SQLITE_TRANSIENT));
    }
    else
    {
        CHECK_SQLITE(sqlite3_bind_null(stmt, 5));
    }
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

void _fill_tseries(sqlite3_stmt *stmt, tseries_t *tseries)
{
    obj_id_t id = sqlite3_column_int64(stmt, 0);
    if (id != tseries->object.id)
        error(DE_BAD_OBJ);
    tseries->eltype = sqlite3_column_int(stmt, 1);
    tseries->elfreq = sqlite3_column_int(stmt, 2);
    tseries->axis.id = sqlite3_column_int64(stmt, 3);
    tseries->nbytes = sqlite3_column_bytes(stmt, 4);
    tseries->value = sqlite3_column_blob(stmt, 4);
}

int sql_load_tseries_value(de_file de, obj_id_t id, tseries_t *tseries)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_load_tseries);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        _fill_tseries(stmt, tseries);
        TRACE_RUN(sql_load_axis(de, tseries->axis.id, &(tseries->axis)));
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error(DE_BAD_OBJ);
    default:
        return rc_error(rc);
    }
}

/**************************************************************/
/* mvtseries */

int sql_store_mvtseries_value(de_file de, obj_id_t id,
                              type_t eltype, frequency_t elfreq,
                              axis_id_t axis1_id, axis_id_t axis2_id,
                              int64_t nbytes, const void *value)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_store_mvtseries);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 2, eltype));
    CHECK_SQLITE(sqlite3_bind_int(stmt, 3, elfreq));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 4, axis1_id));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 5, axis2_id));
    if (value != NULL && nbytes > 0)
    {
        CHECK_SQLITE(sqlite3_bind_blob(stmt, 6, value, nbytes, SQLITE_TRANSIENT));
    }
    else
    {
        CHECK_SQLITE(sqlite3_bind_null(stmt, 6));
    }
    rc = sqlite3_step(stmt);
    return rc == SQLITE_DONE ? DE_SUCCESS : rc_error(rc);
}

void _fill_mvtseries(sqlite3_stmt *stmt, mvtseries_t *mvtseries)
{
    obj_id_t id = sqlite3_column_int64(stmt, 0);
    if (id != mvtseries->object.id)
        error(DE_BAD_OBJ);
    mvtseries->eltype = sqlite3_column_int(stmt, 1);
    mvtseries->elfreq = sqlite3_column_int(stmt, 2);
    mvtseries->axis1.id = sqlite3_column_int64(stmt, 3);
    mvtseries->axis2.id = sqlite3_column_int64(stmt, 4);
    mvtseries->nbytes = sqlite3_column_bytes(stmt, 5);
    mvtseries->value = sqlite3_column_blob(stmt, 5);
}

int sql_load_mvtseries_value(de_file de, obj_id_t id, mvtseries_t *mvtseries)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_load_mvtseries);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, id));
    switch ((rc = sqlite3_step(stmt)))
    {
    case SQLITE_ROW:
        _fill_mvtseries(stmt, mvtseries);
        TRACE_RUN(sql_load_axis(de, mvtseries->axis1.id, &(mvtseries->axis1)));
        TRACE_RUN(sql_load_axis(de, mvtseries->axis2.id, &(mvtseries->axis2)));
        return DE_SUCCESS;
    case SQLITE_DONE:
        return error(DE_BAD_OBJ);
    default:
        return rc_error(rc);
    }
}

int sql_count_objects(de_file de, obj_id_t pid, int64_t *count)
{
    sqlite3_stmt *stmt = _get_statement(de, stmt_count_objects);
    if (stmt == NULL)
        return trace_error();
    int rc;
    CHECK_SQLITE(sqlite3_reset(stmt));
    CHECK_SQLITE(sqlite3_bind_int64(stmt, 1, pid));
    if (SQLITE_ROW == (rc = sqlite3_step(stmt)))
    {
        *count = sqlite3_column_int64(stmt, 0);
        return DE_SUCCESS;
    }
    return rc_error(rc);
}

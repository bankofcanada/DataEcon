#ifndef __DE_SQL_H__
#define __DE_SQL_H__

#include <stdint.h>

#include "file.h"
#include "object.h"
#include "scalar.h"
#include "axis.h"
#include "tseries.h"

/* ========================================================================= */
/* internal */

void _fill_object(sqlite3_stmt *stmt, object_t *object);

/* find the id of an object identified by its parent and its name */
int sql_find_object(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

/* create a new object */
int sql_new_object(de_file de, obj_id_t pid, class_t class, type_t type, const char *name);

/* update insert objects_info for a new object */
int sql_new_object_info(de_file de, obj_id_t id);

/* load object_t data for the given id */
int sql_load_object(de_file de, obj_id_t id, object_t *object);

/* delete an object from the database. If catalog, all children are also removed recursively.*/
int sql_delete_object(de_file de, obj_id_t id);

/* write a row in the attributes table */
int sql_set_attribute(de_file de, int64_t id, const char *name, const char *value);

/* load a row from the attributes table */
int sql_get_attribute(de_file de, int64_t id, const char *name, const char **value);

/* load and format all rows from the attributes table for the given id*/
int sql_get_all_attributes(de_file de, obj_id_t id, const char *delim, int64_t *nattr, const char **names, const char **values);

/* load the info of an object from the objects_info table */
int sql_get_object_info(de_file de, obj_id_t id, const char **fullpath, int64_t *depth, int64_t *created);

/* retrieve the id from objects_info table for a given fullpath */
int sql_find_fullpath(de_file de, const char *fullpath, obj_id_t *id);

/* insert a new row in the scalars table */
int sql_store_scalar_value(de_file de, obj_id_t id, frequency_t freq, int64_t nbytes, const void *value);

/* load data from the scalars table */
int sql_load_scalar_value(de_file de, obj_id_t id, scalar_t *scalar);

/* search for an axis with the given type and data */
int sql_find_axis(de_file de, axis_t *axis);

/* create a new row in the axes table with the given type and data */
int sql_new_axis(de_file de, axis_t *axis);

/* load a row from the axes table with the given id */
int sql_load_axis(de_file de, axis_id_t id, axis_t *axis);

/* create a new row in the tseries table for the given id and data */
int sql_store_tseries_value(de_file de, obj_id_t id, type_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value);

/* load a row from the tseries table with the given id */
int sql_load_tseries_value(de_file de, obj_id_t id, tseries_t *tseries);

#endif

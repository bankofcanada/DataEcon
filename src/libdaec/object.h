#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sqlite3.h>

#include "file.h"

/* ========================================================================= */
/* API */

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

/*****************************************************************************/

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

/* ========================================================================= */
/* internal */

int _new_object(de_file de, obj_id_t pid, class_t class, type_t type,
                const char *name, obj_id_t *id);

/* check if the given string is a valid object name */
bool _check_name(const char *name);

bool check_scalar_type(type_t type);
bool check_tseries_type(type_t type);
bool check_mvtseries_type(type_t type);


#endif

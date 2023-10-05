#ifndef __MVTSERIES_H__
#define __MVTSERIES_H__

#include "file.h"
#include "object.h"
#include "axis.h"

/* ========================================================================= */
/* API */

typedef struct
{
    object_t object;
    type_t eltype;
    frequency_t elfreq;
    axis_t axis1;
    axis_t axis2;
    int64_t nbytes;
    const void *value;
    /* when mvtseries instance is created by a call to de_load_mvtseries the memory for value
    is managed by the library and is valid until the next library call */
} mvtseries_t;
typedef mvtseries_t matrix_t;

/* create a new 1d-array object in a given parent catalog */
int de_store_mvtseries(de_file de, obj_id_t pid, const char *name, type_t type,
                       type_t eltype, frequency_t elfreq,
                       axis_id_t axis1_id, axis_id_t axis2_id,
                       int64_t nbytes, const void *value,
                       obj_id_t *id);

/* load a 2d-array object by name from a given parent catalog */
int de_load_mvtseries(de_file de, obj_id_t id, mvtseries_t *mvtseries);

/* ========================================================================= */
/* internal */

bool check_mvtseries_type(type_t type);

#endif

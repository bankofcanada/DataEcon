#ifndef __NDTSERIES_H__
#define __NDTSERIES_H__

#include "config.h"
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
    int64_t naxes;
    axis_t axis[DE_MAX_AXES];
    int64_t nbytes;
    const void *value;
    /* when ndtseries instance is created by a call to de_load_ndtseries the memory for `value`
    is managed by the library and is valid until the next library call */
} ndtseries_t;
typedef ndtseries_t tensor_t;

/* create a new Nd-array object in a given parent catalog */
DE_API int de_store_ndtseries(de_file de, obj_id_t pid, const char *name, type_t obj_type,
                        type_t eltype, frequency_t elfreq,
                        int64_t naxes, const axis_id_t *axis_ids,
                        int64_t nbytes, const void *value,
                        obj_id_t *id);

/* load a Nd-array object by name from a given parent catalog */
DE_API int de_load_ndtseries(de_file de, obj_id_t id, ndtseries_t *ndtseries);

DE_API int de_load_ndtseries_axis_ids(de_file de, obj_id_t id, axis_id_t *axis_ids);

DE_API int de_load_ndtseries_value(de_file de, obj_id_t id, const void **value);

DE_API int de_load_ndtseries_eltype_elfreq(de_file de, obj_id_t id, type_t *eltype, frequency_t *elfreq);/* ========================================================================= */
/* internal */

bool check_ndtseries_type(type_t type);

#endif

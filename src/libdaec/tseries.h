#ifndef __TSERIES_H__
#define __TSERIES_H__

#include "file.h"
#include "object.h"
#include "axis.h"

/* ========================================================================= */
/* API */

typedef int eltype_t;
int de_pack_eltype(type_t type, frequency_t freq, eltype_t *eltype);
int de_unpack_eltype(eltype_t eltype, type_t *type, frequency_t *freq);

typedef struct
{
    object_t object;
    eltype_t eltype;
    axis_t axis;
    int64_t nbytes;
    const void *value; /* we don't manage the memory for the value */
} tseries_t;
typedef tseries_t vector_t ;

/* create a new 1d-array object in a given parent catalog */
int de_store_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                   eltype_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value,
                   obj_id_t *id);

/* load a 1d-array object by name from a given parent catalog */
int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries);

/* ========================================================================= */
/* internal */

#endif

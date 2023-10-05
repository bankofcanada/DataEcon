#ifndef __TSERIES_H__
#define __TSERIES_H__

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
    axis_t axis;
    int64_t nbytes;
    const void *value;
    /* when tseries instance is created by a call to de_load_tseries the memory for value
    is managed by the library and is valid until the next library call */
} tseries_t;
typedef tseries_t vector_t;

/* create a new 1d-array object in a given parent catalog */
int de_store_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                     type_t eltype, frequency_t elfreq,
                     axis_id_t axis_id, int64_t nbytes, const void *value,
                     obj_id_t *id);

/* load a 1d-array object by name from a given parent catalog */
int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries);

/* ========================================================================= */
/* internal */

bool check_tseries_type(type_t type);
int validate_eltype(type_t obj_type, type_t eltype, frequency_t elfreq);

#endif

#ifndef __SCALAR_H__
#define __SCALAR_H__

#include "error.h"
#include "file.h"
#include "object.h"
#include "catalog.h"

/* ========================================================================= */
/* API */

/* an instance of scalar_t corresponds to a row in the `scalars` table */
typedef struct
{
    object_t object;
    frequency_t frequency;
    int64_t nbytes;
    const void *value;
    /* when scalar instance is created by a call to de_load_scalar the memory for value
    is managed by the library and is valid until the next library call */
} scalar_t;

/* create a new scalar object in a given parent catalog */
int de_store_scalar(de_file de, obj_id_t pid, const char *name, type_t type,
                    frequency_t freq, int64_t nbytes, const void *value,
                    obj_id_t *id);

/* load a scalar object by name from a given parent catalog */
int de_load_scalar(de_file de, obj_id_t id, scalar_t *scalar);

/* ========================================================================= */
/* internal */

bool check_scalar_type(type_t type);

#endif

#ifndef __SCALAR_H__
#define __SCALAR_H__

#include "error.h"
#include "file.h"
#include "object.h"
#include "catalog.h"

/* ========================================================================= */
/* API */

/* an instance of scalar_t corresponds to a row in the `scalars` table*/
typedef struct
{
    object_t object;
    frequency_t frequency;
    int64_t nbytes;
    const void *value; /* we don't manage the memory for the value */
} scalar_t;

/* create a new scalar object in a given parent catalog */
int de_store_scalar(de_file de, obj_id_t pid, const char *name, type_t type,
                  frequency_t freq, int64_t nbytes, const void *value,
                  obj_id_t *id);

/* load a scalar object by name from a given parent catalog */
int de_load_scalar(de_file de, obj_id_t id, scalar_t *scalar);

/* ========================================================================= */
/* internal */

#endif

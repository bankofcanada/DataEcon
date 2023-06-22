#ifndef __CATALOG_H__
#define __CATALOG_H__

#include "file.h"
#include "object.h"

/* ========================================================================= */
/* API */

typedef struct
{
    object_t object;
} catalog_t;

/* create new catalog. return error if catalog already exists */
int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

/* load a catalog object from its id */
int de_load_catalog(de_file de, obj_id_t id, catalog_t *catalog);

/* ========================================================================= */
/* internal */

#endif

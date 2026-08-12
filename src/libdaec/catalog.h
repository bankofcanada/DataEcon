#ifndef __CATALOG_H__
#define __CATALOG_H__

#include "config.h"
#include "file.h"
#include "object.h"

/* ========================================================================= */
/* API */

/* create new catalog. return error if catalog already exists */
DE_API int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id);

/* ========================================================================= */
/* internal */

#endif

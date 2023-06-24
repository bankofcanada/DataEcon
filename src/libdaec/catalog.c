
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "catalog.h"

int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    TRACE_RUN(_new_object(de, pid, class_catalog, type_none, name, id));
    return DE_SUCCESS;
}

int de_load_catalog(de_file de, obj_id_t id, catalog_t *catalog)
{
    if (de == NULL || catalog == NULL)
        return error(DE_NULL);
    TRACE_RUN(de_load_object(de, id, &(catalog->object)));
    return DE_SUCCESS;
}

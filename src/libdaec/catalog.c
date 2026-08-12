
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "error.h"
#include "file.h"
#include "object.h"
#include "catalog.h"

DE_API int de_new_catalog(de_file de, obj_id_t pid, const char *name, obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    TRACE_RUN(new_object(de, pid, class_catalog, type_none, name, id));
    return DE_SUCCESS;
}

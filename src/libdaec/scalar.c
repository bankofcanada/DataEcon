
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "scalar.h"
#include "sql.h"
#include "misc.h"

bool check_scalar_type(type_t type)
{
    return type_integer <= type && type <= type_other_scalar;
}

/* create a new scalar object in a given parent catalog */
int de_store_scalar(de_file de, obj_id_t pid, const char *name, type_t type,
                  frequency_t freq, int64_t nbytes, const void *value,
                  obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    if (!check_scalar_type(type))
        return error(DE_BAD_TYPE);
    obj_id_t _id;
    TRACE_RUN(_new_object(de, pid, class_scalar, type, name, &_id));
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_store_scalar_value(de, _id, freq, nbytes, value));
    return DE_SUCCESS;
}

int de_load_scalar(de_file de, obj_id_t id, scalar_t *scalar)
{
    if (de == NULL || scalar == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, &(scalar->object)));
    if (scalar->object.obj_class != class_scalar)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_scalar_value(de, id, scalar));
    return DE_SUCCESS;
}

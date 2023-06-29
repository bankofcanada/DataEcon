
#include "error.h"
#include "file.h"
#include "object.h"
#include "axis.h"
#include "tseries.h"
#include "sql.h"

bool check_tseries_type(type_t type)
{
    return type_vector <= type && type <= type_other_1d;
}

/* create a new 1d-array object in a given parent catalog */
int de_store_tseries(de_file de, obj_id_t pid, const char *name, type_t type,
                   type_t eltype, axis_id_t axis_id, int64_t nbytes, const void *value,
                   obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    if (!check_tseries_type(type))
        return error(DE_BAD_TYPE);
    while (true)
    {
        if (check_scalar_type(eltype))
            break;
        if (type == type_range && eltype == type_none)
            break;
        return error(DE_BAD_TYPE);
    }
    obj_id_t _id;
    TRACE_RUN(_new_object(de, pid, class_tseries, type, name, &_id));
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_store_tseries_value(de, _id, eltype, axis_id, nbytes, value));
    return DE_SUCCESS;
}

/* load a 1d-array object by name from a given parent catalog */
int de_load_tseries(de_file de, obj_id_t id, tseries_t *tseries)
{
    if (de == NULL || tseries == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, &(tseries->object)));
    if (tseries->object.class != class_tseries)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_tseries_value(de, id, tseries));
    return DE_SUCCESS;
}

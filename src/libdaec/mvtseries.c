
#include "error.h"
#include "file.h"
#include "object.h"
#include "axis.h"
#include "mvtseries.h"
#include "sql.h"

bool check_mvtseries_type(type_t type)
{
    return type_matrix <= type && type <= type_other_2d;
}

/* create a new 2d-array object in a given parent catalog */
int de_store_mvtseries(de_file de, obj_id_t pid, const char *name, type_t type,
                       type_t eltype, axis_id_t axis1_id, axis_id_t axis2_id,
                       int64_t nbytes, const void *value,
                       obj_id_t *id)
{
    if (de == NULL || name == NULL)
        return error(DE_NULL);
    if (!check_mvtseries_type(type))
        return error(DE_BAD_TYPE);
    if (!check_scalar_type(eltype))
        return error(DE_BAD_TYPE);

    obj_id_t _id;
    TRACE_RUN(_new_object(de, pid, class_mvtseries, type, name, &_id));
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_store_mvtseries_value(de, _id, eltype, axis1_id, axis2_id, nbytes, value));
    return DE_SUCCESS;
}

/* load a 2d-array object by name from a given parent catalog */
int de_load_mvtseries(de_file de, obj_id_t id, mvtseries_t *mvtseries)
{
    if (de == NULL || mvtseries == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, &(mvtseries->object)));
    if (mvtseries->object.obj_class != class_mvtseries)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_mvtseries_value(de, id, mvtseries));
    return DE_SUCCESS;
}

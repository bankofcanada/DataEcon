
#include "config.h"
#include "error.h"
#include "file.h"
#include "object.h"
#include "axis.h"
#include "tseries.h"
#include "mvtseries.h"
#include "sql.h"

bool check_ndtseries_type(type_t type)
{
    return type_tensor <= type && type <= type_other_nd;
}

/* create a new Nd-array object in a given parent catalog */
int de_store_ndtseries(de_file de, obj_id_t pid, const char *name, type_t obj_type,
                       type_t eltype, frequency_t elfreq,
                       int64_t naxes, const axis_id_t *axis_ids,
                       int64_t nbytes, const void *value,
                       obj_id_t *id)
{
    if (de == NULL || name == NULL || axis_ids == NULL)
        return error(DE_NULL);
    if (!check_ndtseries_type(obj_type))
        return error(DE_BAD_TYPE);
    TRACE_RUN(validate_eltype(obj_type, eltype, elfreq));
    if ((naxes < 0) || (naxes > DE_MAX_AXES))
        return error(DE_BAD_NUM_AXES);

    obj_id_t _id;
    TRACE_RUN(_new_object(de, pid, class_ndtseries, obj_type, name, &_id));
    if (id != NULL)
        *id = _id;
    TRACE_RUN(sql_store_ndtseries_value(de, _id, eltype, elfreq, nbytes, value));
    for (int64_t n = 0; n < naxes; ++n)
        TRACE_RUN(sql_store_ndaxes(de, _id, n, axis_ids[n]));
    return DE_SUCCESS;
}

/* load a Nd-array object by name from a given parent catalog */
int de_load_ndtseries(de_file de, obj_id_t id, ndtseries_t *ndtseries)
{
    if (de == NULL || ndtseries == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_object(de, id, &(ndtseries->object)));
    if (ndtseries->object.obj_class != class_ndtseries)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_ndtseries_value(de, id, ndtseries));
    return DE_SUCCESS;
}

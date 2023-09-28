
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
    if (eltype == type_date)
        return error(DE_BAD_ELTYPE_DATE);
    if ((type == type_range) != (eltype == type_none))
        return error(DE_BAD_ELTYPE_NONE);
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
    if (tseries->object.obj_class != class_tseries)
        return error(DE_BAD_CLASS);
    TRACE_RUN(sql_load_tseries_value(de, id, tseries));
    return DE_SUCCESS;
}

int de_pack_eltype(type_t type, frequency_t freq, eltype_t *eltype)
{
    if (eltype == NULL)
        return error(DE_NULL);
    if ((type == type_date) != (freq >= freq_unit))
        return error(DE_BAD_FREQ) ;
    *eltype = type == type_date ? freq : type;
    return DE_SUCCESS;
}

int de_unpack_eltype(eltype_t eltype, type_t *type, frequency_t *freq)
{
    if (type == NULL || freq == NULL)
        return error(DE_NULL);
    if (eltype > type_other_scalar)
    {
        *type = type_date;
        *freq = eltype;
    }
    else
    {
        *type = eltype;
        *freq = freq_none;
    }
    return DE_SUCCESS;
}

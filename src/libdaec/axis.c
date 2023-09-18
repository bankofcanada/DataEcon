
#include "error.h"
#include "file.h"
#include "axis.h"
#include "sql.h"

int _get_axis(de_file de, axis_t *axis)
{
    int rc = sql_find_axis(de, axis);
    if (rc == DE_SUCCESS)
        return DE_SUCCESS;
    if (rc != DE_AXIS_DNE)
        return trace_error();
    de_clear_error();
    TRACE_RUN(sql_new_axis(de, axis));
    return DE_SUCCESS;
}

int de_axis_plain(de_file de, int64_t length, axis_id_t *id)
{
    if (de == NULL || id == NULL)
        return error(DE_NULL);
    axis_t axis;
    axis.ax_type = axis_plain;
    axis.length = length;
    axis.frequency = freq_none;
    axis.first = 0;
    axis.names = NULL;
    TRACE_RUN(_get_axis(de, &axis));
    *id = axis.id;
    return DE_SUCCESS;
}

int de_axis_range(de_file de, int64_t length, frequency_t frequency, int64_t first, axis_id_t *id)
{
    if (de == NULL || id == NULL)
        return error(DE_NULL);
    axis_t axis;
    axis.ax_type = axis_range;
    axis.length = length;
    axis.frequency = frequency;
    axis.first = first;
    axis.names = NULL;
    TRACE_RUN(_get_axis(de, &axis));
    *id = axis.id;
    return DE_SUCCESS;
}

int de_axis_names(de_file de, int64_t length, const char *names, axis_id_t *id)
{
    if (de == NULL || id == NULL)
        return error(DE_NULL);
    axis_t axis;
    axis.ax_type = axis_names;
    axis.length = length;
    axis.frequency = freq_none;
    axis.first = 0;
    axis.names = names;
    TRACE_RUN(_get_axis(de, &axis));
    *id = axis.id;
    return DE_SUCCESS;
}

int de_load_axis(de_file de, axis_id_t id, axis_t *axis)
{
    if (de == NULL || axis == NULL)
        return error(DE_NULL);
    TRACE_RUN(sql_load_axis(de, id, axis));
    return DE_SUCCESS;
}

#ifndef __AXIS_H__
#define __AXIS_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "file.h"
#include "object.h"

/* ========================================================================= */
/* API */

typedef int64_t axis_id_t;

typedef enum
{
    axis_plain = 0, /* a simple numerical range, e.g. 1:5 */
    axis_range,     /* a range of dates, e.g. 2020Q1:2025Q4 */
    axis_names,     /* an ordered list of variable names, e.g. ("color", "length", "shape") */
} axis_type_t;

typedef struct
{
    axis_id_t id;
    axis_type_t type;
    int64_t length;
    frequency_t frequency;
    /* at most one of these is valid, depending on type */
    int64_t first;
    const char *names;
} axis_t;

/* create or find an axis of the given type */
int de_axis_plain(de_file de, int64_t length, axis_id_t *id);
int de_axis_range(de_file de, int64_t length, frequency_t frequency, int64_t first, axis_id_t *id);
int de_axis_names(de_file de, int64_t length, const char *names, axis_id_t *id);

/* load an axis from its id */
int de_load_axis(de_file de, axis_id_t id, axis_t *axis);

/* ========================================================================= */
/* internal */

#endif


#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "common.h"

#include "daec.h"

struct frequencies_map
{
    frequency_t freq_code;
    const char *freq_name;
};

static struct frequencies_map FREQUENCIES_MAP[] = {
    {freq_none, "none"},
    {freq_daily, "daily"},
    {freq_bdaily, "bdaily"},
    {freq_weekly, "weekly"},
    {freq_weekly_mon, "weekly_mon"},
    {freq_weekly_tue, "weekly_tue"},
    {freq_weekly_wed, "weekly_wed"},
    {freq_weekly_thu, "weekly_thu"},
    {freq_weekly_fri, "weekly_fri"},
    {freq_weekly_sat, "weekly_sat"},
    {freq_weekly_sun, "weekly_sun"},
    {freq_monthly, "monthly"},
    {freq_quarterly, "quarterly"},
    {freq_quarterly_jan, "quarterly_jan"},
    {freq_quarterly_feb, "quarterly_feb"},
    {freq_quarterly_mar, "quarterly_mar"},
    {freq_quarterly_apr, "quarterly_apr"},
    {freq_quarterly_may, "quarterly_may"},
    {freq_quarterly_jun, "quarterly_jun"},
    {freq_quarterly_jul, "quarterly_jul"},
    {freq_quarterly_aug, "quarterly_aug"},
    {freq_quarterly_sep, "quarterly_sep"},
    {freq_quarterly_oct, "quarterly_oct"},
    {freq_quarterly_nov, "quarterly_nov"},
    {freq_quarterly_dec, "quarterly_dec"},
    {freq_halfyearly, "halfyearly"},
    {freq_halfyearly_jan, "halfyearly_jan"},
    {freq_halfyearly_feb, "halfyearly_feb"},
    {freq_halfyearly_mar, "halfyearly_mar"},
    {freq_halfyearly_apr, "halfyearly_apr"},
    {freq_halfyearly_may, "halfyearly_may"},
    {freq_halfyearly_jun, "halfyearly_jun"},
    {freq_halfyearly_jul, "halfyearly_jul"},
    {freq_halfyearly_aug, "halfyearly_aug"},
    {freq_halfyearly_sep, "halfyearly_sep"},
    {freq_halfyearly_oct, "halfyearly_oct"},
    {freq_halfyearly_nov, "halfyearly_nov"},
    {freq_halfyearly_dec, "halfyearly_dec"},
    {freq_yearly, "yearly"},
    {freq_yearly_jan, "yearly_jan"},
    {freq_yearly_feb, "yearly_feb"},
    {freq_yearly_mar, "yearly_mar"},
    {freq_yearly_apr, "yearly_apr"},
    {freq_yearly_may, "yearly_may"},
    {freq_yearly_jun, "yearly_jun"},
    {freq_yearly_jul, "yearly_jul"},
    {freq_yearly_aug, "yearly_aug"},
    {freq_yearly_sep, "yearly_sep"},
    {freq_yearly_oct, "yearly_oct"},
    {freq_yearly_nov, "yearly_nov"},
    {freq_yearly_dec, "yearly_dec"},
    {-1, NULL}};

int _find_frequency_code(const char *text)
{
    int i = 0;
    while (true)
    {
        struct frequencies_map *F = &FREQUENCIES_MAP[i];
        if (F->freq_name == NULL || strcmp(text, F->freq_name) == 0)
            return F->freq_code;
        i += 1;
    }
}

const char *_find_frequency_text(frequency_t freq)
{
    int i = 0;
    while (true)
    {
        struct frequencies_map *F = &FREQUENCIES_MAP[i];
        if (F->freq_code == -1 || F->freq_code == freq)
            return F->freq_name;
        i += 1;
    }
}

/****************************************************************************/

struct types_map
{
    type_t type_code;
    const char *type_name;
};
static struct types_map TYPES_MAP[] = {
    {type_none, "none"},
    {type_integer, "integer"},
    {type_signed, "signed"},
    {type_unsigned, "unsigned"},
    {type_date, "date"},
    {type_float, "float"},
    {type_complex, "complex"},
    {type_string, "string"},
    {type_other_scalar, "other_scalar"},
    {type_vector, "vector"},
    {type_range, "range"},
    {type_tseries, "tseries"},
    {type_other_1d, "other_1d"},
    {type_matrix, "matrix"},
    {type_mvtseries, "mvtseries"},
    {type_other_2d, "other_2d"},
    {type_any, "any"},
    {-1, NULL}};

int _find_type_code(const char *text)
{
    int i = 0;
    while (true)
    {
        struct types_map *T = &TYPES_MAP[i];
        if (T->type_name == NULL || strcmp(text, T->type_name) == 0)
            return T->type_code;
        i += 1;
    }
}
const char *_find_type_text(type_t obj_type)
{
    int i = 0;
    while (true)
    {
        struct types_map *T = &TYPES_MAP[i];
        if (T->type_code == -1 || T->type_code == obj_type)
            return T->type_name;
        i += 1;
    }
}

/****************************************************************************/

struct classes_map
{
    class_t class_code;
    const char *class_name;
};
static struct classes_map CLASSES_MAP[] = {
    {class_catalog, "catalog"},
    {class_scalar, "scalar"},
    {class_tseries, "tseries"},
    {class_mvtseries, "mvtseries"},
    {class_any, "any"},
    {-1, NULL}};

int _find_class_code(const char *text)
{
    int i = 0;
    while (true)
    {
        struct classes_map *C = &CLASSES_MAP[i];
        if (C->class_name == NULL || strcmp(text, C->class_name) == 0)
            return C->class_code;
        i += 1;
    }
}
const char *_find_class_text(class_t obj_class)
{
    int i = 0;
    while (true)
    {
        struct classes_map *C = &CLASSES_MAP[i];
        if (C->class_code == -1 || C->class_code == obj_class)
            return C->class_name;
        i += 1;
    }
}

/****************************************************************************/

void print_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void print_de_error()
{
    static char message[1024];
    de_error(message, sizeof message - 1);
    print_error(message);
}

/****************************************************************************/

int snprintf_value(char *restrict buffer, size_t bufsz, type_t obj_type, frequency_t freq, int64_t nbytes, const void *value)
{
    /* dispatch according to obj_type */
    switch (obj_type)
    {
    case type_integer:
        return snprintf_integer(buffer, bufsz, nbytes, value);
    case type_unsigned:
        return snprintf_unsigned(buffer, bufsz, nbytes, value);
    case type_float:
        return snprintf_float(buffer, bufsz, nbytes, value);
    case type_date:
        return snprintf_date(buffer, bufsz, freq, nbytes, value);
    case type_string:
        return snprintf_string(buffer, bufsz, nbytes, value);
    default:
        print_error("Cannot print value of type %s(%d).", _find_type_text(obj_type), obj_type);
    }
    return 0;
}

int snprintf_integer(char *restrict buffer, size_t bufsz, int64_t nbytes, const void *value)
{
    switch (nbytes)
    {
    case 8:
        return snprintf(buffer, bufsz, "%" PRId64, *(int64_t *)value);
    case 4:
        return snprintf(buffer, bufsz, "%" PRId32, *(int32_t *)value);
    case 2:
        return snprintf(buffer, bufsz, "%" PRId16, *(int16_t *)value);
    case 1:
        return snprintf(buffer, bufsz, "%" PRId8, *(int8_t *)value);
    default:
        print_error("Cannot print integer with %" PRId64 " bytes.\n", nbytes);
    }
    return 0;
}

int snprintf_unsigned(char *restrict buffer, size_t bufsz, int64_t nbytes, const void *value)
{
    switch (nbytes)
    {
    case 8:
        return snprintf(buffer, bufsz, "%" PRIu64, *(uint64_t *)value);
    case 4:
        return snprintf(buffer, bufsz, "%" PRIu32, *(uint32_t *)value);
    case 2:
        return snprintf(buffer, bufsz, "%" PRIu16, *(uint16_t *)value);
    case 1:
        return snprintf(buffer, bufsz, "%" PRIu8, *(uint8_t *)value);
    default:
        print_error("Cannot print unsigned integer with %" PRId64 " bytes.\n", nbytes);
    }
    return 0;
}

int snprintf_float(char *restrict buffer, size_t bufsz, int64_t nbytes, const void *value)
{
    switch (nbytes)
    {
    case 8:
        return snprintf(buffer, bufsz, "%lg", *(double *)value);
    case 4:
        return snprintf(buffer, bufsz, "%g", *(float *)value);
    default:
        print_error("Cannot print a floating point number with %" PRId64 " bytes.\n", nbytes);
    }
    return 0;
}

static date_fmt_t date_fmt;

date_fmt_t get_date_fmt()
{
    return date_fmt;
}

date_fmt_t set_date_fmt(date_fmt_t fmt)
{
    date_fmt_t old = date_fmt;
    date_fmt = fmt;
    return old;
}

bool _freq_is_yp(frequency_t freq)
{
    return (freq & (freq_monthly | freq_quarterly | freq_halfyearly | freq_yearly)) != 0;
}

bool _freq_is_ymd(frequency_t freq)
{
    return freq == freq_daily || freq == freq_bdaily || (freq & freq_weekly) != 0;
}

int snprintf_date(char *restrict buffer, size_t bufsz, frequency_t freq, int64_t nbytes, const void *value)
{
    if ((date_fmt == date_fmt_ymd) || ((date_fmt == date_fmt_auto) && _freq_is_ymd(freq)))
    {
        int32_t Y;
        uint32_t M, D;
        if (DE_SUCCESS != de_unpack_calendar_date(freq, *(date_t *)value, &Y, &M, &D))
        {
            print_de_error();
            return 0;
        }
        return snprintf(buffer, bufsz, "%" PRId32 "-%02" PRIu32 "-%02" PRIu32, Y, M, D);
    }
    else if ((date_fmt == date_fmt_yp) || ((date_fmt == date_fmt_auto) && _freq_is_yp(freq)))
    {
        int32_t Y;
        uint32_t P;
        if (DE_SUCCESS != de_unpack_year_period_date(freq, *(date_t *)value, &Y, &P))
        {
            print_de_error();
            return 0;
        }
        return snprintf(buffer, bufsz, "%" PRId32 "-%02" PRIu32, Y, P);
    }
    else if (freq == freq_unit)
    {
        return snprintf(buffer, bufsz, "%" PRId64, *(int64_t *)value);
    }
    else
    {
        print_error("Cannot print date with frequency %s(%d).", _find_frequency_text(freq), freq);
        return 0;
    }
}

int snprintf_string(char *restrict buffer, size_t bufsz, int64_t nbytes, const void *value)
{
    return snprintf(buffer, bufsz, "\"%s\"", (char *)value);
}

/****************************************************************************/

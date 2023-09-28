#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include "daec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int _find_frequency_code(const char *text);
    const char *_find_frequency_text(frequency_t freq);

    int _find_type_code(const char *text);
    const char *_find_type_text(type_t obj_type);

    int _find_class_code(const char *text);
    const char *_find_class_text(class_t obj_class);

    void print_error(const char *message, ...);
    void print_de_error();

    void print_value(FILE *F, type_t obj_type, frequency_t freq, int64_t nbytes, const void *value);
    void print_integer(FILE *F, int64_t nbytes, const void *value);
    void print_unsigned(FILE *F, int64_t nbytes, const void *value);
    void print_float(FILE *F, int64_t nbytes, const void *value);
    void print_string(FILE *F, int64_t nbytes, const void *value);
    void print_date(FILE *F, frequency_t freq, int64_t nbytes, const void *value);

    typedef enum date_fmt_enum
    {
        date_fmt_auto,
        date_fmt_ymd,
        date_fmt_yp
    } date_fmt_t;

    date_fmt_t get_date_fmt();
    date_fmt_t set_date_fmt(date_fmt_t date_fmt);

#ifdef __cplusplus
}
#endif

#endif


#include "daec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static de_file de;
static int checks;

void fail(const char *file, int line, const char *message)
{
    printf("Tests passed: %d\n", checks);
    printf("Fail at %s:%d: %s\n", file, line, message);
    de_close(de);
    exit(EXIT_FAILURE);
}

#define FAIL(msg) fail(__FILE__, __LINE__, msg)
#define FAIL_IF(cond, msg)                 \
    {                                      \
        if (cond)                          \
            fail(__FILE__, __LINE__, msg); \
        ++checks;                          \
    }

void check(int rc, int expected, const char *file, int line)
{
    static char message[4096];
    snprintf(message, sizeof message, "\nExpected: DE(%d)\nReceived: ", expected);
    int len = strlen(message);
    char *p = message + strlen(message);
    de_error_source(p, sizeof message - len - 1);
    if (rc == expected)
    {
        ++checks;
        de_clear_error();
    }
    else
    {
        fail(file, line, message);
    }
}

#define CHECK_SCALAR(scalar, id, type, frequency, value) check_scalar(scalar, id, type, frequency, value, __FILE__, __LINE__)
void check_scalar(scalar_t scalar, int64_t id, type_t type, frequency_t frequency, void *value, const char *file, int line)
{
    if (scalar.object.id != id)
        fail(file, line, "Scalar id doesn't match.");
    if (scalar.object.class != class_scalar)
        fail(file, line, "Scalar class doesn't match.");
    if (scalar.object.type != type)
        fail(file, line, "Scalar type doesn't match.");
    if (scalar.frequency != frequency)
        fail(file, line, "Scalar frequency doesn't match.");
    if (scalar.value == NULL && value != NULL)
        fail(file, line, "Scalar value is unexpectedly NULL.");
    if (scalar.value != NULL && value == NULL)
        fail(file, line, "Scalar value is unexpectedly not NULL.");
    if (scalar.value == NULL && scalar.nbytes != 0)
        fail(file, line, "Scalar nbytes is not 0 while value is NULL.");
    if (scalar.value != NULL && scalar.nbytes <= 0)
        fail(file, line, "Scalar nbytes is not positive while value is not NULL.");
    if (scalar.value != NULL && value != NULL && memcmp(scalar.value, value, scalar.nbytes) != 0)
        fail(file, line, "Scalar value doesn't match.");
    ++checks;
}

#define CHECK_AXIS(axis, id, type, length, frequency, first, names) check_axis(axis, id, type, length, frequency, first, names, __FILE__, __LINE__)
void check_axis(axis_t axis, axis_id_t id, axis_type_t type, int64_t length, frequency_t frequency, int64_t first, const char *names, const char *file, int line)
{
    if (id >= 0 && id != axis.id)
        fail(file, line, "Axis id doesn't match");
    if (axis.type != type)
        fail(file, line, "Axis type doesn't match");
    if (axis.frequency != frequency)
        fail(file, line, "Axis frequency doesn't match");
    if (axis.first != first)
        fail(file, line, "Axis first doesn't match");
    if (axis.names == NULL && names == NULL)
    {
        ++checks;
        return;
    }
    if (axis.names == NULL || names == NULL || strcmp(axis.names, names) != 0)
        fail(file, line, "Axis names don't match");
    ++checks;
}

#define CHECK_TSERIES(tseries, id, type, eltype, axis, value) check_tseries(tseries, id, type, eltype, axis, value, __FILE__, __LINE__)
void check_tseries(tseries_t tseries, obj_id_t id, type_t type, type_t eltype, axis_id_t axis, void *value, const char *file, int line)
{
    if (tseries.object.id != id)
        fail(file, line, "tseries id doesn't match.");
    if (tseries.object.class != class_tseries)
        fail(file, line, "tseries class doesn't match.");
    if (tseries.object.type != type)
        fail(file, line, "tseries type doesn't match.");
    if (tseries.eltype != eltype)
        fail(file, line, "tseries eltype doesn't match.");
    if (tseries.axis.id != axis)
        fail(file, line, "tseries axis doesn't match.");
    if (tseries.value == NULL && value != NULL)
        fail(file, line, "tseries value is unexpectedly NULL.");
    if (tseries.value != NULL && value == NULL)
        fail(file, line, "tseries value is unexpectedly not NULL.");
    if (tseries.value == NULL && tseries.nbytes != 0)
        fail(file, line, "tseries nbytes is not 0 while value is NULL.");
    if (tseries.value != NULL && tseries.nbytes <= 0)
        fail(file, line, "tseries nbytes is not positive while value is not NULL.");
    if (tseries.value != NULL && value != NULL && memcmp(tseries.value, value, tseries.nbytes) != 0)
        fail(file, line, "tseries value doesn't match.");
    ++checks;
}

#define CHECK_MVTSERIES(mvtseries, id, type, eltype, axis1, axis2, value) \
    check_mvtseries(mvtseries, id, type, eltype, axis1, axis2, value, __FILE__, __LINE__)

void check_mvtseries(mvtseries_t mvtseries, obj_id_t id, type_t type, type_t eltype,
                     axis_id_t axis1, axis_id_t axis2, void *value, const char *file, int line)
{
    if (mvtseries.object.id != id)
        fail(file, line, "mvtseries id doesn't match.");
    if (mvtseries.object.class != class_mvtseries)
        fail(file, line, "mvtseries class doesn't match.");
    if (mvtseries.object.type != type)
        fail(file, line, "mvtseries type doesn't match.");
    if (mvtseries.eltype != eltype)
        fail(file, line, "mvtseries eltype doesn't match.");
    if (mvtseries.axis1.id != axis1)
        fail(file, line, "mvtseries axis1 doesn't match.");
    if (mvtseries.axis2.id != axis2)
        fail(file, line, "mvtseries axis2 doesn't match.");
    if (mvtseries.value == NULL && value != NULL)
        fail(file, line, "mvtseries value is unexpectedly NULL.");
    if (mvtseries.value != NULL && value == NULL)
        fail(file, line, "mvtseries value is unexpectedly not NULL.");
    if (mvtseries.value == NULL && mvtseries.nbytes != 0)
        fail(file, line, "mvtseries nbytes is not 0 while value is NULL.");
    if (mvtseries.value != NULL && mvtseries.nbytes <= 0)
        fail(file, line, "mvtseries nbytes is not positive while value is not NULL.");
    if (mvtseries.value != NULL && value != NULL && memcmp(mvtseries.value, value, mvtseries.nbytes) != 0)
        fail(file, line, "mvtseries value doesn't match.");
    ++checks;
}

#define CHECK_SUCCESS(x) check(x, DE_SUCCESS, __FILE__, __LINE__)
#define CHECK(x, e) check(x, e, __FILE__, __LINE__)

#define CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, N, y, m, d, p) \
    check_pack_year_period_unpack_calendar(fr, N, y, m, d, p, __FILE__, __LINE__)

void check_pack_year_period_unpack_calendar(frequency_t fr, date_t N,
                                            int32_t y, uint32_t m, uint32_t d, uint32_t p,
                                            const char *file, int line)
{
    date_t date;
    int32_t Y;
    uint32_t P, M, D;
    check(de_pack_year_period_date(fr, y, p, &date), DE_SUCCESS, file, line);
    if (date != N)
        fail(file, line, "N does not match");
    check(de_unpack_year_period_date(fr, date, &Y, &P), DE_SUCCESS, file, line);
    if (Y != y)
        fail(file, line, "year 1 does not match");
    if (P != p)
        fail(file, line, "period does not match");
    check(de_unpack_calendar_date(fr, date, &Y, &M, &D), DE_SUCCESS, file, line);
    if (Y != y)
        fail(file, line, "year 2 does not match");
    if (M != m)
        fail(file, line, "month does not match");
    if (D != d)
        fail(file, line, "day does not match");
}

int main(void)
{

    CHECK(strcmp(DE_VERSION, de_version()), 0);

    CHECK(de_open("./path/does/not/exist/file.daec", &de), 14); /* SQLITE_CANTOPEN = 14 */

    const static char fname[] = "test.daec";
    unlink(fname);
    CHECK_SUCCESS(de_open(fname, &de));
    CHECK_SUCCESS(de_close(de));
    CHECK_SUCCESS(de_open(fname, &de));

    /* load object from id */
    object_t object;
    CHECK(de_load_object(NULL, 0, &object), DE_NULL);
    CHECK(de_load_object(de, 0, NULL), DE_NULL);
    CHECK_SUCCESS(de_load_object(de, 0, &object));
    FAIL_IF(object.id != 0 || object.pid != 0 || object.class != class_catalog || object.type != type_none || strcmp(object.name, "/") != 0, "")

    /* find object id from parent-id and name */
    int64_t id, id1;
    CHECK(de_find_object(NULL, 0, "/", &id), DE_NULL);
    CHECK(de_find_object(de, 0, NULL, &id), DE_NULL);
    CHECK_SUCCESS(de_find_object(de, 0, "/", NULL));
    CHECK_SUCCESS(de_find_object(de, 0, "/", &id));
    FAIL_IF(id != 0, "");

    /* new catalog */
    id = -1;
    CHECK_SUCCESS(de_new_catalog(de, 0, "boyan", &id));
    FAIL_IF(id <= 0, "Expected id > 0");

    /* error if already exists */
    id1 = -1;
    CHECK(de_new_catalog(de, 0, "boyan", &id1), DE_EXISTS);
    CHECK_SUCCESS(de_find_object(de, 0, "boyan", &id1));
    FAIL_IF(id != id1, "Create and find id don't match");

    /* error if name is invalid */
    id = -1;
    CHECK(de_new_catalog(de, 0, NULL, &id), DE_NULL);
    CHECK(de_new_catalog(de, 0, "   ", &id), DE_BAD_NAME);
    CHECK(de_new_catalog(de, 0, "", &id), DE_BAD_NAME);
    CHECK(de_new_catalog(de, 0, "hello/world", &id), DE_BAD_NAME);

    {
        /* nested catalogs */
        int64_t id_hello, id_world, id_super;
        CHECK_SUCCESS(de_new_catalog(de, 0, "hello", &id_hello));
        CHECK_SUCCESS(de_new_catalog(de, id_hello, "world", &id_world));
        CHECK_SUCCESS(de_new_catalog(de, id_world, "super", &id_super));

        const char *fp;
        int64_t depth;
        CHECK(de_get_object_info(NULL, 0, &fp, &depth, NULL), DE_NULL);
        CHECK(de_get_object_info(de, 0, NULL, NULL, NULL), DE_NULL);
        CHECK(de_get_object_info(de, 20, &fp, NULL, NULL), DE_OBJ_DNE);

        CHECK_SUCCESS(de_get_object_info(de, 0, &fp, &depth, NULL));
        FAIL_IF(strcmp(fp, "/") != 0 || depth != 0, "fullpath of root");
        CHECK_SUCCESS(de_get_object_info(de, id_hello, &fp, &depth, NULL));
        FAIL_IF(strcmp(fp, "/hello") != 0 || depth != 1, "fullpath of hello");
        CHECK_SUCCESS(de_get_object_info(de, id_world, &fp, &depth, NULL));
        FAIL_IF(strcmp(fp, "/hello/world") != 0 || depth != 2, "fullpath of world");
        CHECK_SUCCESS(de_get_object_info(de, id_super, &fp, &depth, NULL));
        FAIL_IF(strcmp(fp, "/hello/world/super") != 0 || depth != 3, "fullpath of super");

        int64_t id;
        CHECK(de_find_fullpath(NULL, "/", &id), DE_NULL);
        CHECK(de_find_fullpath(de, NULL, &id), DE_NULL);
        CHECK(de_find_fullpath(de, "/", NULL), DE_NULL);
        CHECK_SUCCESS(de_find_fullpath(de, "/", &id));
        FAIL_IF(id != 0, "find fullpath of \"/\"");
        CHECK_SUCCESS(de_find_fullpath(de, "/hello", &id));
        FAIL_IF(id != id_hello, "find fullpath of \"/hello\"");
        CHECK_SUCCESS(de_find_fullpath(de, "/hello/world", &id));
        FAIL_IF(id != id_world, "find fullpath \"/hello/world\"");
        CHECK_SUCCESS(de_find_fullpath(de, "/hello/world/super", &id));
        FAIL_IF(id != id_super, "find fullpath \"/hello/world/super\"");
        CHECK(de_find_fullpath(de, "/hello/does/not/exist", &id), DE_OBJ_DNE);

        /* delete object works and deletes catalogs recursively */
        CHECK(de_delete_object(NULL, id_hello), DE_NULL);
        CHECK(de_delete_object(de, 0), DE_DEL_ROOT);

        CHECK_SUCCESS(de_delete_object(de, id_hello));
        CHECK(de_find_object(de, 0, "hello", &id), DE_OBJ_DNE);
        CHECK(de_load_object(de, id_hello, &object), DE_OBJ_DNE);
        CHECK(de_load_object(de, id_world, &object), DE_OBJ_DNE);
        CHECK(de_load_object(de, id_super, &object), DE_OBJ_DNE);
    }

    /* test scalars */
    {
        int64_t id_scalars;
        CHECK_SUCCESS(de_new_catalog(de, 0, "scalars", &id_scalars));

        double one = 1;
        float two = 2;
        int64_t three = 3;
        uint32_t four = 4;
        char five[] = "five";
        int64_t q2020Q1 = 2020 * 4 + 1;
        CHECK(de_store_scalar(NULL, id_scalars, "fail", type_float, freq_none, sizeof one, &one, NULL), DE_NULL);
        CHECK(de_store_scalar(de, id_scalars, NULL, type_float, freq_none, sizeof one, &one, NULL), DE_NULL);
        CHECK(de_store_scalar(de, id_scalars, "fail", type_none, freq_none, sizeof one, &one, NULL), DE_BAD_TYPE);
        CHECK(de_store_scalar(de, id_scalars, "fail", type_tseries, freq_none, sizeof one, &one, NULL), DE_BAD_TYPE);
        CHECK(de_store_scalar(de, id_scalars, "fail", type_mvtseries, freq_none, sizeof one, &one, NULL), DE_BAD_TYPE);

        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "one", type_float, freq_none, sizeof one, &one, &id));
        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "two", type_float, freq_none, sizeof two, &two, NULL));
        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "three", type_signed, freq_none, sizeof three, &three, NULL));
        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "four", type_unsigned, freq_none, sizeof four, &four, NULL));
        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "five", type_string, freq_none, sizeof five + 1, five, NULL));
        CHECK_SUCCESS(de_store_scalar(de, id_scalars, "qdate", type_date, freq_quarterly, sizeof q2020Q1, &q2020Q1, NULL));

        scalar_t scalar;
        CHECK_SUCCESS(de_find_object(de, id_scalars, "one", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_float, freq_none, &one);

        CHECK(de_load_scalar(NULL, id, &scalar), DE_NULL);
        CHECK(de_load_scalar(de, id, NULL), DE_NULL);
        CHECK(de_load_scalar(de, id_scalars, &scalar), DE_BAD_CLASS);

        CHECK_SUCCESS(de_find_object(de, id_scalars, "two", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_float, freq_none, &two);
        CHECK_SUCCESS(de_find_object(de, id_scalars, "three", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_signed, freq_none, &three);
        CHECK_SUCCESS(de_find_object(de, id_scalars, "four", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_unsigned, freq_none, &four);
        CHECK_SUCCESS(de_find_object(de, id_scalars, "five", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_string, freq_none, five);
        CHECK_SUCCESS(de_find_object(de, id_scalars, "qdate", &id));
        CHECK_SUCCESS(de_load_scalar(de, id, &scalar));
        CHECK_SCALAR(scalar, id, type_date, freq_quarterly, &q2020Q1);
        // printf("Loaded scalar 'one':\n");
        // printf("    id=%ld, pid=%ld, class=%d, type=%d, name='%s', frequency=%d, nbytes=%ld, value=%p, *value=%g\n",
        //     scalar.object.id, scalar.object.pid, scalar.object.class, scalar.object.type, scalar.object.name,
        //         scalar.frequency, scalar.nbytes,scalar.value, scalar.value ? *(double*)scalar.value : NAN);
    }

    /* test attributes */
    {
        obj_id_t _id;
        int val = 77;
        CHECK_SUCCESS(de_store_scalar(de, 0, "attr_test", type_signed, freq_none, sizeof val, &val, &_id));
        CHECK(de_set_attribute(de, _id, NULL, NULL), DE_NULL);
        CHECK(de_set_attribute(NULL, _id, "greeting", NULL), DE_NULL);
        CHECK_SUCCESS(de_set_attribute(de, _id, "greeting", NULL));
        const char *value;
        const char *name;
        int64_t num;
        CHECK(de_get_attribute(NULL, _id, "greeting", &value), DE_NULL);
        CHECK(de_get_attribute(de, _id, "greeting", NULL), DE_NULL);
        CHECK(de_get_attribute(de, _id, NULL, &value), DE_NULL);
        CHECK(de_get_attribute(de, _id, "not_greeting", &value), DE_MIS_ATTR);
        CHECK_SUCCESS(de_get_attribute(de, _id, "greeting", &value));
        FAIL_IF(value != NULL, "Not NULL attribute value");
        CHECK_SUCCESS(de_set_attribute(de, _id, "greeting", "hello"));
        CHECK_SUCCESS(de_get_attribute(de, _id, "greeting", &value));
        FAIL_IF(value == NULL, "Update attribute value");
        FAIL_IF(strcmp(value, "hello") != 0, "Update attribute value");
        CHECK_SUCCESS(de_set_attribute(de, _id, "summary", "what do you think you're doing?"));
        CHECK(de_get_all_attributes(NULL, _id, ",", &num, &name, &value), DE_NULL);
        CHECK(de_get_all_attributes(de, _id, NULL, &num, &name, &value), DE_NULL);
        CHECK(de_get_all_attributes(de, _id, ",", NULL, &name, &value), DE_NULL);
        CHECK_SUCCESS(de_get_all_attributes(de, _id, ",", &num, &name, &value));
        FAIL_IF(num != 2, "");
        FAIL_IF(strcmp(name, "greeting,summary") != 0, "");
        FAIL_IF(strcmp(value, "hello,what do you think you're doing?") != 0, "");
        /* object exists, but has no attributes - return num = 0 */
        CHECK_SUCCESS(de_get_all_attributes(de, 0, ",", &num, &name, &value));
        FAIL_IF(num != 0 || name != NULL || value != NULL, "");
        /* object does not exist - DE_OBJ_DNE*/
        CHECK(de_get_all_attributes(de, -1, ",", &num, &name, &value), DE_OBJ_DNE);
    }

    /* test axes */
    {
        axis_t axis;
        axis_id_t _id;
        axis_id_t id;

        CHECK(de_axis_plain(NULL, 5, &_id), DE_NULL);
        CHECK(de_axis_plain(de, 5, NULL), DE_NULL);

        CHECK_SUCCESS(de_axis_plain(de, 5, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_plain, 5, freq_none, 0, NULL);

        CHECK(de_load_axis(NULL, _id, &axis), DE_NULL);
        CHECK(de_load_axis(de, _id, NULL), DE_NULL);

        id = axis.id;
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_plain(de, 5, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, id, axis_plain, 5, freq_none, 0, NULL);

        CHECK(de_axis_range(NULL, 5, freq_quarterly_jan, 8081, &_id), DE_NULL);
        CHECK(de_axis_range(de, 5, freq_quarterly_jan, 8081, NULL), DE_NULL);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_range(de, 5, freq_quarterly_jan, 8081, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        FAIL_IF(axis.id == id, "This is strange");
        CHECK_AXIS(axis, _id, axis_range, 5, freq_quarterly_jan, 8081, NULL);

        id = axis.id;
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_range(de, 5, freq_quarterly_jan, 8081, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, id, axis_range, 5, freq_quarterly_jan, 8081, NULL);

        const char *names = "color\nshape\nsize";

        CHECK(de_axis_names(NULL, 3, names, &_id), DE_NULL);
        CHECK(de_axis_names(de, 3, names, NULL), DE_NULL);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_names(de, 3, names, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_names, 3, freq_none, 0, names);

        id = axis.id;
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_names(de, 3, names, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, id, axis_names, 3, freq_none, 0, names);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_plain(de, 11, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_plain, 11, freq_none, 0, NULL);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_range(de, 8, freq_monthly, 8081, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_range, 8, freq_monthly, 8081, NULL);

        id = axis.id;
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_range(de, 8, freq_daily, 8081, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        FAIL_IF(id == axis.id, "Axis range ignores frequency.");
        CHECK_AXIS(axis, _id, axis_range, 8, freq_daily, 8081, NULL);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_range(de, 8, freq_monthly, 22, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        FAIL_IF(id == axis.id, "Axis range ignores first date.");
        CHECK_AXIS(axis, _id, axis_range, 8, freq_monthly, 22, NULL);

        const char *names4 = "color\nshape\nsize\nhello";
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_names(de, 4, names4, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_names, 4, freq_none, 0, names4);

        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_names(de, 3, names4 + sizeof "color", &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        CHECK_AXIS(axis, _id, axis_names, 3, freq_none, 0, names4 + sizeof "color");

        id = axis.id;
        memset(&axis, 0, sizeof axis);
        CHECK_SUCCESS(de_axis_names(de, 3, names, &_id));
        CHECK_SUCCESS(de_load_axis(de, _id, &axis));
        FAIL_IF(id == _id, "Axis names doesn't compare names");
        CHECK_AXIS(axis, _id, axis_names, 3, freq_none, 0, names);
    }

    /* test tseries */
    {
        obj_id_t id_tseries;
        CHECK_SUCCESS(de_new_catalog(de, 0, "tseries", &id_tseries));

        axis_id_t ax;
        obj_id_t _id;
        tseries_t ts;

        /* plain range */
        CHECK_SUCCESS(de_axis_plain(de, 11, &ax));
        CHECK(de_store_tseries(de, id_tseries, "rng_plain", type_integer, type_integer, ax, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK(de_store_tseries(de, id_tseries, "rng_plain", type_tseries, type_none, ax, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK(de_store_tseries(de, id_tseries, "rng_plain", type_mvtseries, type_float, ax, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK_SUCCESS(de_store_tseries(de, id_tseries, "rng_plain", type_range, type_none, ax, 0, NULL, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        CHECK_TSERIES(ts, _id, type_range, type_none, ax, NULL);

        /* dates range */
        CHECK_SUCCESS(de_axis_range(de, 6, freq_halfyearly, 4041, &ax));
        CHECK_SUCCESS(de_store_tseries(de, id_tseries, "rng_dates", type_range, type_none, ax, 0, NULL, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        CHECK_TSERIES(ts, _id, type_range, type_none, ax, NULL);

        double dvals[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
        CHECK_SUCCESS(de_axis_plain(de, sizeof dvals / sizeof dvals[0], &ax));
        CHECK_SUCCESS(de_store_tseries(de, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        CHECK_TSERIES(ts, _id, type_tseries, type_float, ax, dvals);

        CHECK(de_store_tseries(de, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_EXISTS);
        CHECK(de_store_tseries(NULL, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_NULL);
        CHECK(de_store_tseries(de, id_tseries, NULL, type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_NULL);
        CHECK(de_load_tseries(NULL, _id, &ts), DE_NULL);
        CHECK(de_load_tseries(de, _id, NULL), DE_NULL);
        CHECK(de_load_tseries(de, id_tseries, &ts), DE_BAD_CLASS);

        float fvals[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
        CHECK_SUCCESS(de_axis_plain(de, sizeof fvals / sizeof fvals[0], &ax));
        FAIL_IF(ax != ts.axis.id, "Duplicate axis.");
        CHECK_SUCCESS(de_store_tseries(de, id_tseries, "ts_float", type_tseries, type_float, ax, sizeof fvals, fvals, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        CHECK_TSERIES(ts, _id, type_tseries, type_float, ax, fvals);

        const char *svals[5] = {"one", "two", "three", "four", "five"};
        const char *svals1[8] = {svals[0], svals[0], svals[0], svals[0], svals[0], svals[0], svals[0], svals[0]};
        char buffer[1024];
        int64_t bufsize;
        CHECK(de_pack_strings(NULL, 5, NULL, &bufsize), DE_NULL);
        CHECK(de_pack_strings(svals, 5, NULL, NULL), DE_NULL);
        bufsize = 0;
        CHECK_SUCCESS(de_pack_strings(svals, 5, NULL, &bufsize));
        FAIL_IF(bufsize == 0, "Bufsize didn't update");
        bufsize = sizeof buffer;
        CHECK(de_pack_strings(svals, 5, NULL, &bufsize), DE_NULL);
        bufsize = 5;
        CHECK(de_pack_strings(svals, 5, buffer, &bufsize), DE_SHORT_BUF);
        bufsize = sizeof buffer;
        CHECK_SUCCESS(de_pack_strings(svals, 5, buffer, &bufsize));
        FAIL_IF(bufsize == sizeof buffer, "Bufsize didn't update");
        CHECK_SUCCESS(de_store_tseries(de, id_tseries, "ts_string", type_tseries, type_string, ax, bufsize, buffer, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        FAIL_IF(ts.nbytes != bufsize, "nbytes doesn't match.");
        CHECK_TSERIES(ts, _id, type_tseries, type_string, ax, buffer);

        CHECK(de_unpack_strings(NULL, ts.nbytes, svals1, 5), DE_NULL);
        CHECK(de_unpack_strings(buffer, ts.nbytes, NULL, 5), DE_NULL);
        CHECK(de_unpack_strings(buffer, ts.nbytes, svals1, 8), DE_ARG);
        for (int i = 0; i < 5; ++i)
            FAIL_IF(
                (intptr_t)buffer > (intptr_t)svals1[i] || (intptr_t)buffer + ts.nbytes <= (intptr_t)svals1[i],
                "de_unpack_strings gave pointer outside buffer");
        for (int i = 5; i < 8; ++i)
            FAIL_IF(
                svals1[i] != NULL,
                "de_unpack_strings didn't clean up pointers");
        CHECK_SUCCESS(de_unpack_strings(buffer, ts.nbytes, svals1, ts.axis.length));
        for (int i = 0; i < 5; ++i)
            FAIL_IF(
                strcmp(svals[i], svals1[i]) != 0,
                "String TSeries loaded values don't match");
    }

    /* test mvtseries */
    {
        obj_id_t cata;
        CHECK_SUCCESS(de_new_catalog(de, 0, "mvtseries", &cata));

        axis_id_t ax1;
        axis_id_t ax2;
        obj_id_t _id;
        mvtseries_t data;

        /* plain range */
        CHECK(de_store_mvtseries(de, cata, "fail", type_integer, type_integer, ax1, ax2, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK(de_store_mvtseries(de, cata, "fail", type_tseries, type_integer, ax1, ax2, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK(de_store_mvtseries(de, cata, "fail", type_mvtseries, type_none, ax1, ax2, 0, NULL, &_id), DE_BAD_TYPE);
        CHECK(de_store_mvtseries(de, cata, "fail", type_mvtseries, type_tseries, ax1, ax2, 0, NULL, &_id), DE_BAD_TYPE);

        CHECK(de_store_mvtseries(NULL, cata, "fail", type_matrix, type_integer, ax1, ax1, 0, NULL, &_id), DE_NULL);
        CHECK(de_store_mvtseries(de, cata, NULL, type_matrix, type_integer, ax1, ax1, 0, NULL, &_id), DE_NULL);

        CHECK(de_load_mvtseries(NULL, _id, &data), DE_NULL);
        CHECK(de_load_mvtseries(de, _id, NULL), DE_NULL);
        CHECK(de_load_mvtseries(de, cata, &data), DE_BAD_CLASS);

        CHECK_SUCCESS(de_axis_plain(de, 0, &ax1));
        CHECK_SUCCESS(de_store_mvtseries(de, cata, "empty", type_matrix, type_integer, ax1, ax1, 0, NULL, &_id));

        CHECK_SUCCESS(de_load_mvtseries(de, _id, &data));
        CHECK_MVTSERIES(data, _id, type_matrix, type_integer, ax1, ax1, NULL);

        double values[3][2] = {{1, 2}, {3, 4}, {5, 6}};
        CHECK_SUCCESS(de_axis_range(de, 2, freq_monthly, 550, &ax1));
        char col_names[] = "apple\norange\ntomato";
        CHECK_SUCCESS(de_axis_names(de, 3, col_names, &ax2));

        axis_t ax_nms;
        CHECK_SUCCESS(de_load_axis(de, ax2, &ax_nms));
        CHECK_AXIS(ax_nms, ax2, axis_names, 3, freq_none, 0, col_names);

        CHECK_SUCCESS(de_store_mvtseries(de, cata, "two_by_three", type_mvtseries, type_float, ax1, ax2, sizeof values, values, &_id));
        CHECK_SUCCESS(de_load_mvtseries(de, _id, &data));
        CHECK_MVTSERIES(data, _id, type_mvtseries, type_float, ax1, ax2, values);
    }

    /* test search and list */
    {
        CHECK_SUCCESS(de_finalize_search(NULL)); // harmless no-op

        de_search search;
        CHECK(de_list_catalog(NULL, 0, &search), DE_NULL);
        CHECK(de_list_catalog(de, 0, NULL), DE_NULL);
        CHECK(de_search_catalog(NULL, 0, NULL, type_any, class_any, &search), DE_NULL);
        CHECK(de_search_catalog(de, 0, NULL, type_any, class_any, NULL), DE_NULL);

        CHECK(de_list_catalog(de, 0, &search), DE_SUCCESS);
        object_t object;
        int rc;
        // int count = 0;
        // const char *path;
        while (DE_SUCCESS == (rc = de_next_object(search, &object)))
        {
            FAIL_IF(object.id == object.pid, "search yields parent");
            FAIL_IF(object.pid != 0, "not in the catalog");
            // ++count;
            // CHECK_SUCCESS(de_get_object_info(de, object.id, &path, NULL, NULL));
            // printf("id = %d, fullpath = %s\n", (int)object.id, path);
        }
        CHECK(rc, DE_NO_OBJ);
        CHECK_SUCCESS(de_finalize_search(search));

        int64_t val = 0;
        // obj_id_t id;
        CHECK_SUCCESS(de_store_scalar(de, 0, "scal1", type_integer, freq_none, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_store_scalar(de, 0, "scal2", type_float, freq_none, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_store_scalar(de, 0, "scal3", type_string, freq_none, sizeof val, &val, NULL));
        axis_id_t aid;
        CHECK_SUCCESS(de_axis_plain(de, 1, &aid));
        CHECK_SUCCESS(de_store_tseries(de, 0, "ts1", type_vector, type_integer, aid, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_store_tseries(de, 0, "ts2", type_vector, type_float, aid, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_store_tseries(de, 0, "ts3", type_vector, type_string, aid, sizeof val, &val, NULL));
        for (class_t class = class_catalog; class <= class_tseries; ++class)
        {
            int count = 0;
            CHECK_SUCCESS(de_search_catalog(de, 0, NULL, type_any, class, &search));
            while (DE_SUCCESS == (rc = de_next_object(search, &object)))
            {
                FAIL_IF(object.class != class, "class does not match");
                ++count;
            }
            FAIL_IF(count == 0, "Nothing found");
            CHECK(rc, DE_NO_OBJ);
            CHECK_SUCCESS(de_finalize_search(search));
        }
        {
            int count = 0;
            CHECK_SUCCESS(de_search_catalog(de, 0, "scal*", type_any, class_any, &search));
            while (DE_SUCCESS == (rc = de_next_object(search, &object)))
            {
                FAIL_IF(strncmp(object.name, "scal", 4) != 0, "wildcard didn't work");
                ++count;
            }
            FAIL_IF(count == 0, "Nothing found");
            CHECK(rc, DE_NO_OBJ);
            CHECK_SUCCESS(de_finalize_search(search));
        }
        {
            int count = 0;
            CHECK_SUCCESS(de_search_catalog(de, 0, "ts*", type_any, class_any, &search));
            while (DE_SUCCESS == (rc = de_next_object(search, &object)))
            {
                FAIL_IF(strncmp(object.name, "ts", 2) != 0, "wildcard didn't work");
                ++count;
            }
            FAIL_IF(count == 0, "Nothing found");
            CHECK(rc, DE_NO_OBJ);
            CHECK_SUCCESS(de_finalize_search(search));
        }
        {
            int count = 0;
            CHECK_SUCCESS(de_search_catalog(de, 0, NULL, type_integer, class_any, &search));
            while (DE_SUCCESS == (rc = de_next_object(search, &object)))
            {
                FAIL_IF(object.type != type_integer, "type filter didn't work");
                ++count;
            }
            FAIL_IF(count == 0, "Nothing found");
            CHECK(rc, DE_NO_OBJ);
            CHECK_SUCCESS(de_finalize_search(search));
        }
        {
            int count = 0;
            CHECK_SUCCESS(de_search_catalog(de, 0, "scal*", type_integer, class_any, &search));
            while (DE_SUCCESS == (rc = de_next_object(search, &object)))
            {
                FAIL_IF(object.type != type_integer, "type filter didn't work");
                ++count;
            }
            FAIL_IF(count == 0, "Nothing found");
            CHECK(rc, DE_NO_OBJ);
            CHECK_SUCCESS(de_finalize_search(search));
        }
    }

    {
        de_search search;
        CHECK_SUCCESS(de_list_catalog(de, 0, &search));
        CHECK(de_close(de), 5); /* SQLITE_BUSY */
        CHECK_SUCCESS(de_finalize_search(search));
    }

    {
        object_t obj;
        de_search search;
        CHECK_SUCCESS(de_list_catalog(de, 0, &search));
        CHECK_SUCCESS(de_next_object(search, &obj));
        CHECK_SUCCESS(de_finalize_search(search));

        CHECK_SUCCESS(de_truncate(de));
        CHECK_SUCCESS(de_list_catalog(de, 0, &search));
        CHECK(de_next_object(search, &obj), DE_NO_OBJ);
        CHECK_SUCCESS(de_finalize_search(search));

        CHECK_SUCCESS(de_close(de));
        CHECK_SUCCESS(de_open(fname, &de));
        CHECK_SUCCESS(de_list_catalog(de, 0, &search));
        CHECK(de_next_object(search, &obj), DE_NO_OBJ);
        CHECK_SUCCESS(de_finalize_search(search));
    }

    CHECK_SUCCESS(de_close(de));

    // Test dates packing and unpacking
    {
        date_t d;
        int32_t Y;
        uint32_t M, D, P;
        frequency_t fr;

        /**********************************************************************/
        /*  test errors */

        fr = freq_daily;

        CHECK(de_pack_year_period_date(fr, 0, 0, NULL), DE_NULL);
        CHECK(de_pack_calendar_date(fr, 0, 0, 0, NULL), DE_NULL);

        CHECK(de_unpack_year_period_date(fr, d, &Y, NULL), DE_NULL);
        CHECK(de_unpack_year_period_date(fr, d, NULL, &P), DE_NULL);
        CHECK(de_unpack_calendar_date(fr, d, NULL, NULL, NULL), DE_NULL);
        CHECK(de_unpack_calendar_date(fr, d, &Y, NULL, NULL), DE_NULL);
        CHECK(de_unpack_calendar_date(fr, d, NULL, &M, NULL), DE_NULL);
        CHECK(de_unpack_calendar_date(fr, d, NULL, NULL, &D), DE_NULL);

        CHECK(de_pack_calendar_date(fr, -39000, 0, 0, &d), DE_RANGE);
        CHECK(de_pack_calendar_date(fr, 39000, 0, 0, &d), DE_RANGE);
        CHECK(de_pack_calendar_date(fr, 0, 15, 0, &d), DE_RANGE);

        /**********************************************************************/
        /* test de_{pack,unpack}_calendar_date */
        /* these don't work with YP frequencies */

        for (D = 1; D <= 31; D++)
        {
            CHECK_SUCCESS(de_pack_calendar_date(freq_daily, 1, 1, D, &d));
            FAIL_IF(d != D, "problem packing daily frequency");
        }
        for (D = 1; D <= 5; D++)
        {
            CHECK_SUCCESS(de_pack_calendar_date(freq_bdaily, 1, 1, D, &d));
            FAIL_IF(d != D, "problem packing daily frequency");
        }
        for (D = 6; D <= 7; D++)
            CHECK(de_pack_calendar_date(freq_bdaily, 1, 1, D, &d), DE_INEXACT);
        for (D = 8; D <= 12; D++)
        {
            CHECK_SUCCESS(de_pack_calendar_date(freq_bdaily, 1, 1, D, &d));
            FAIL_IF(d != D - 2, "problem packing business daily frequency");
        }

        for (fr = freq_weekly_sun0; fr <= freq_weekly_sun7; ++fr)
        {
            CHECK_SUCCESS(de_pack_calendar_date(fr, 1, 1, 1, &d));
            FAIL_IF(d != 1, "problem packing with weekly frequency date");
        }
        for (D = 1; D <= 7; ++D)
        {
            fr = freq_weekly_sun0;
            CHECK_SUCCESS(de_pack_calendar_date(fr, 1, 1, D, &d));
            FAIL_IF(d != 1, "problem packing with weekly frequency date");
        }

        fr = freq_daily;
        CHECK_SUCCESS(de_pack_calendar_date(fr, 2023, 8, 16, &d));
        d += 1;
        CHECK_SUCCESS(de_unpack_calendar_date(fr, d, &Y, &M, &D));
        FAIL_IF(Y != 2023 || M != 8 || D != 17, "daily");

        fr = freq_bdaily;
        CHECK_SUCCESS(de_pack_calendar_date(fr, 2023, 7, 27, &d));
        d += 1; /* next business day after thursday is friday */
        CHECK_SUCCESS(de_unpack_calendar_date(fr, d, &Y, &M, &D));
        FAIL_IF(Y != 2023 || M != 7 || D != 28, "bdaily");
        d += 1; /* next business day after friday is monday */
        CHECK_SUCCESS(de_unpack_calendar_date(fr, d, &Y, &M, &D));
        FAIL_IF(Y != 2023 || M != 7 || D != 31, "bdaily");

        for (fr = freq_weekly_sun0; fr <= freq_weekly_sat; ++fr)
        {
            /* 2023-8-13 is a sunday */
            CHECK_SUCCESS(de_pack_calendar_date(fr, 2023, 8, 13, &d));
            CHECK_SUCCESS(de_unpack_calendar_date(fr, d, &Y, &M, &D));
            FAIL_IF(Y != 2023 || M != 8 || D != 13 + fr % 16, "weekly")
        }

        /**********************************************************************/
        /*  test de_pack_year_period_date */

        for (fr = freq_yearly_jan; fr <= freq_yearly_dec; ++fr)
        {
            CHECK_SUCCESS(de_pack_year_period_date(fr, 0, 1, &d));
            FAIL_IF(d != 0, "yearly");
            CHECK_SUCCESS(de_unpack_year_period_date(fr, d, &Y, &P));
            FAIL_IF(Y != 0 || P != 1, "yearly");
        }
        for (fr = freq_halfyearly_jan; fr <= freq_halfyearly_dec; ++fr)
        {
            CHECK_SUCCESS(de_pack_year_period_date(fr, 0, 1, &d));
            FAIL_IF(d != 0, "halfyearly");
            CHECK_SUCCESS(de_unpack_year_period_date(fr, d, &Y, &P));
            FAIL_IF(Y != 0 || P != 1, "halfyearly");
        }
        for (fr = freq_quarterly_jan; fr <= freq_quarterly_dec; ++fr)
        {
            CHECK_SUCCESS(de_pack_year_period_date(fr, 0, 1, &d));
            FAIL_IF(d != 0, "quarterly");
            CHECK_SUCCESS(de_unpack_year_period_date(fr, d, &Y, &P));
            FAIL_IF(Y != 0 || P != 1, "quarterly");
        }
        {
            fr = freq_monthly;
            CHECK_SUCCESS(de_pack_year_period_date(fr, 0, 1, &d));
            FAIL_IF(d != 0, "monthly");
            CHECK_SUCCESS(de_unpack_year_period_date(fr, d, &Y, &P));
            FAIL_IF(Y != 0 || P != 1, "monthly");
        }
        {
            fr = freq_daily;
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 6, 1, 1, 6, 6);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 24160, 67, 2, 23, 54);
        }
        {
            fr = freq_bdaily;
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 6, 1, 1, 8, 6);

            /* Jan 1st year 1 is encoded as 1 */
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 1, 1, 1, 1, 1);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 2, 1, 1, 2, 2);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 0, 0, 12, 29, 260);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, -259, 0, 1, 3, 1);

            /* some random dates */
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 17258, 67, 2, 23, 38);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 17686, 68, 10, 15, 206);
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, -11048, -42, 8, 26, 170);
        }
        for (fr = freq_weekly_mon; fr <= freq_weekly_sun7; ++fr)
        {
            CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 2, 1, 1, 8 + fr - freq_weekly_mon, 2);

            if (fr == freq_weekly_tue)
                CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 3452 + (fr <= freq_weekly_tue), 67, 3, 1, 9);
            else if (fr == freq_weekly_mon)
                CHECK_PACK_YEAR_PERIOD_UNPACK_CALENDAR(fr, 3452 + (fr <= freq_weekly_tue), 67, 2, 23 + (14 + fr - freq_weekly_wed) % 7, 8 + (fr <= freq_weekly_tue));
        }
    }

    printf("All %d tests passed.\n", checks);
    return EXIT_SUCCESS;
}


#include "daec.h"

// #include "libdaec/error.h"
// #include "libdaec/file.h"
// #include "libdaec/object.h"
// #include "libdaec/catalog.h"
// #include "libdaec/scalar.h"
// #include "libdaec/axis.h"
// #include "libdaec/tseries.h"
// #include "libdaec/search.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static de_file de;
static int checks;

void fail(const char *message, const char *file, int line)
{
    printf("Tests passed: %d\n", checks);
    printf("Fail at %s:%d: %s\n", file, line, message);
    de_close(de);
    exit(EXIT_FAILURE);
}

#define FAIL(msg) fail(msg, __FILE__, __LINE__)
#define FAIL_IF(cond, msg)                 \
    {                                      \
        if (cond)                          \
            fail(msg, __FILE__, __LINE__); \
        ++checks;                          \
    }

void check(int rc, int expected, const char *file, int line)
{
    static char message[4096];
    if (rc == expected)
    {
        ++checks;
        de_clear_error();
        return;
    }
    snprintf(message, sizeof message, "\nExpected: DE(%d)\nReceived: ", expected);
    int len = strlen(message);
    char *p = message + strlen(message);
    de_error_source(p, sizeof message - len - 1);
    fail(message, file, line);
}

#define CHECK_SCALAR(scalar, id, type, frequency, value) check_scalar(scalar, id, type, frequency, value, __FILE__, __LINE__)
void check_scalar(scalar_t scalar, int64_t id, type_t type, frequency_t frequency, void *value, const char *file, int line)
{
    if (scalar.object.id != id)
        fail("Scalar id doesn't match.", file, line);
    if (scalar.object.class != class_scalar)
        fail("Scalar class doesn't match.", file, line);
    if (scalar.object.type != type)
        fail("Scalar type doesn't match.", file, line);
    if (scalar.frequency != frequency)
        fail("Scalar frequency doesn't match.", file, line);
    if (scalar.value == NULL && value != NULL)
        fail("Scalar value is unexpectedly NULL.", file, line);
    if (scalar.value != NULL && value == NULL)
        fail("Scalar value is unexpectedly not NULL.", file, line);
    if (scalar.value == NULL && scalar.nbytes != 0)
        fail("Scalar nbytes is not 0 while value is NULL.", file, line);
    if (scalar.value != NULL && scalar.nbytes <= 0)
        fail("Scalar nbytes is not positive while value is not NULL.", file, line);
    if (scalar.value != NULL && value != NULL && memcmp(scalar.value, value, scalar.nbytes) != 0)
        fail("Scalar value doesn't match.", file, line);
    ++checks;
}

#define CHECK_AXIS(axis, id, type, length, frequency, first, names) check_axis(axis, id, type, length, frequency, first, names, __FILE__, __LINE__)
void check_axis(axis_t axis, axis_id_t id, axis_type_t type, int64_t length, frequency_t frequency, int64_t first, const char *names, const char *file, int line)
{
    if (id >= 0 && id != axis.id)
        fail("Axis id doesn't match", file, line);
    if (axis.type != type)
        fail("Axis type doesn't match", file, line);
    if (axis.frequency != frequency)
        fail("Axis frequency doesn't match", file, line);
    if (axis.first != first)
        fail("Axis first doesn't match", file, line);
    if (axis.names == NULL && names == NULL)
    {
        ++checks;
        return;
    }
    if (axis.names == NULL || names == NULL || strcmp(axis.names, names) != 0)
        fail("Axis names don't match", file, line);
    ++checks;
}

#define CHECK_TSERIES(tseries, id, type, eltype, axis, value) check_tseries(tseries, id, type, eltype, axis, value, __FILE__, __LINE__)
void check_tseries(tseries_t tseries, obj_id_t id, type_t type, type_t eltype, axis_id_t axis, void *value, const char *file, int line)
{
    if (tseries.object.id != id)
        fail("TSeries id doesn't match.", file, line);
    if (tseries.object.class != class_tseries)
        fail("TSeries class doesn't match.", file, line);
    if (tseries.object.type != type)
        fail("TSeries type doesn't match.", file, line);
    if (tseries.eltype != eltype)
        fail("TSeries eltype doesn't match.", file, line);
    if (tseries.axis.id != axis)
        fail("TSeries axis doesn't match.", file, line);
    if (tseries.value == NULL && value != NULL)
        fail("TSeries value is unexpectedly NULL.", file, line);
    if (tseries.value != NULL && value == NULL)
        fail("TSeries value is unexpectedly not NULL.", file, line);
    if (tseries.value == NULL && tseries.nbytes != 0)
        fail("TSeries nbytes is not 0 while value is NULL.", file, line);
    if (tseries.value != NULL && tseries.nbytes <= 0)
        fail("TSeries nbytes is not positive while value is not NULL.", file, line);
    if (tseries.value != NULL && value != NULL && memcmp(tseries.value, value, tseries.nbytes) != 0)
        fail("TSeries value doesn't match.", file, line);
    ++checks;
}

#define CHECK_SUCCESS(x) check(x, DE_SUCCESS, __FILE__, __LINE__)
#define CHECK(x, e) check(x, e, __FILE__, __LINE__)

int main()
{
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

        /* load object works for a catalog */
        catalog_t hello;
        CHECK_SUCCESS(de_load_catalog(de, id_hello, &hello));
        FAIL_IF(hello.object.id != id_hello, "Id create and load id don't match");
        CHECK(de_load_catalog(de, id_hello, NULL), DE_NULL);

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
        CHECK(de_new_scalar(NULL, id_scalars, "one", type_float, freq_none, sizeof one, &one, NULL), DE_NULL);
        CHECK(de_new_scalar(de, id_scalars, NULL, type_float, freq_none, sizeof one, &one, NULL), DE_NULL);

        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "one", type_float, freq_none, sizeof one, &one, &id));
        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "two", type_float, freq_none, sizeof two, &two, NULL));
        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "three", type_signed, freq_none, sizeof three, &three, NULL));
        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "four", type_unsigned, freq_none, sizeof four, &four, NULL));
        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "five", type_string, freq_none, sizeof five + 1, five, NULL));
        CHECK_SUCCESS(de_new_scalar(de, id_scalars, "qdate", type_date, freq_quarterly, sizeof q2020Q1, &q2020Q1, NULL));

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
        CHECK_SUCCESS(de_new_scalar(de, 0, "attr_test", type_signed, freq_none, sizeof val, &val, &_id));
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

        double dvals[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
        CHECK_SUCCESS(de_axis_plain(de, sizeof dvals / sizeof dvals[0], &ax));
        CHECK_SUCCESS(de_new_tseries(de, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id));
        CHECK_SUCCESS(de_load_tseries(de, _id, &ts));
        CHECK_TSERIES(ts, _id, type_tseries, type_float, ax, dvals);

        CHECK(de_new_tseries(de, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_EXISTS);
        CHECK(de_new_tseries(NULL, id_tseries, "ts_double", type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_NULL);
        CHECK(de_new_tseries(de, id_tseries, NULL, type_tseries, type_float, ax, sizeof dvals, dvals, &_id), DE_NULL);
        CHECK(de_load_tseries(NULL, _id, &ts), DE_NULL);
        CHECK(de_load_tseries(de, _id, NULL), DE_NULL);
        CHECK(de_load_tseries(de, id_tseries, &ts), DE_BAD_CLASS);

        float fvals[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
        CHECK_SUCCESS(de_axis_plain(de, sizeof fvals / sizeof fvals[0], &ax));
        FAIL_IF(ax != ts.axis.id, "Duplicate axis.");
        CHECK_SUCCESS(de_new_tseries(de, id_tseries, "ts_float", type_tseries, type_float, ax, sizeof fvals, fvals, &_id));
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
        CHECK_SUCCESS(de_new_tseries(de, id_tseries, "ts_string", type_tseries, type_string, ax, bufsize, buffer, &_id));
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
            FAIL_IF(object.pid != 0, "not in the catalog");
            // ++count;
            // CHECK_SUCCESS(de_get_object_info(de, object.id, &path, NULL, NULL));
            // printf("id = %d, fullpath = %s\n", (int)object.id, path);
        }
        CHECK(rc, DE_NO_OBJ);
        CHECK_SUCCESS(de_finalize_search(search));

        int64_t val = 0;
        // obj_id_t id;
        CHECK_SUCCESS(de_new_scalar(de, 0, "scal1", type_integer, freq_none, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_new_scalar(de, 0, "scal2", type_float, freq_none, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_new_scalar(de, 0, "scal3", type_string, freq_none, sizeof val, &val, NULL));
        axis_id_t aid;
        CHECK_SUCCESS(de_axis_plain(de, 1, &aid));
        CHECK_SUCCESS(de_new_tseries(de, 0, "ts1", type_vector, type_integer, aid, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_new_tseries(de, 0, "ts2", type_vector, type_float, aid, sizeof val, &val, NULL));
        CHECK_SUCCESS(de_new_tseries(de, 0, "ts3", type_vector, type_string, aid, sizeof val, &val, NULL));
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

    CHECK_SUCCESS(de_close(de));

    printf("All %d tests passed.\n", checks);
    return EXIT_SUCCESS;
}

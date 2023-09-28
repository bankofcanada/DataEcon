

#include <string>
#include <iostream>
#include <fstream>
#include <complex>
#include <iomanip>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "daec.h"

using namespace std;

static std::ofstream D;
static std::ofstream M;
static de_file de;

bool export_scalars = export_scalars;

void print_usage(const char *program)
{
    std::cerr << "Usage: " << program << " infile.daec out.data.csv out.manifest.csv\n";
    std::cerr << "    `outfile` should be just the basename of the output file.\n";
    std::cerr << "    Two .csv file will be written - outfile.data.csv and outfile.manifest.csv.\n";
    std::cerr << std::endl;
}

int print_de_error()
{
    char message[1024];
    int rc = de_error(message, sizeof message);
    std::cerr << message << std::endl;
    return rc;
}

void print_header()
{
    if (M.is_open())
    {
        M << "name,class,type,frequency" << '\n';
    }
    if (D.is_open())
    {
        D << "date,name,value" << '\n';
    }
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, const type_t obj_type)
{
    out << '"';
    out << (obj_type == type_none ? "none" : obj_type == type_integer    ? "integer"
                                         : obj_type == type_unsigned     ? "unsigned"
                                         : obj_type == type_date         ? "date"
                                         : obj_type == type_float        ? "float"
                                         : obj_type == type_complex      ? "complex"
                                         : obj_type == type_string       ? "string"
                                         : obj_type == type_other_scalar ? "other_scalar"
                                         : obj_type == type_vector       ? "vector"
                                         : obj_type == type_range        ? "range"
                                         : obj_type == type_tseries      ? "tseries"
                                         : obj_type == type_other_1d     ? "other_1d"
                                         : obj_type == type_matrix       ? "matrix"
                                         : obj_type == type_mvtseries    ? "mvtseries"
                                         : obj_type == type_other_2d     ? "other_2d"
                                         : obj_type == type_any          ? "any"
                                                                         : "unknown");
    return out;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, const class_t obj_class)
{
    out << '"';
    switch (obj_class)
    {
    case class_catalog:
        out << "catalog";
        break;
    case class_scalar:
        out << "scalar";
        break;
    case class_tseries:
        out << "tseries";
        break;
    case class_mvtseries:
        out << "mvtseries";
        break;
    default:
        out << "class " << obj_class;
    }
    return out << '"';
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &out, const frequency_t freq)
{
    out << '"';
    out << (freq == freq_none ? "none" : freq == freq_unit         ? "unit"
                                     : freq == freq_daily          ? "daily"
                                     : freq == freq_bdaily         ? "bdaily"
                                     : freq == freq_weekly         ? "weekly"
                                     : freq == freq_weekly_sun0    ? "weekly_sun0"
                                     : freq == freq_weekly_mon     ? "weekly_mon"
                                     : freq == freq_weekly_tue     ? "weekly_tue"
                                     : freq == freq_weekly_wed     ? "weekly_wed"
                                     : freq == freq_weekly_thu     ? "weekly_thu"
                                     : freq == freq_weekly_fri     ? "weekly_fri"
                                     : freq == freq_weekly_sat     ? "weekly_sat"
                                     : freq == freq_weekly_sun7    ? "weekly_sun7"
                                     : freq == freq_weekly_sun     ? "weekly_sun"
                                     : freq == freq_monthly        ? "monthly"
                                     : freq == freq_quarterly      ? "quarterly"
                                     : freq == freq_quarterly_jan  ? "quarterly_jan"
                                     : freq == freq_quarterly_feb  ? "quarterly_feb"
                                     : freq == freq_quarterly_mar  ? "quarterly_mar"
                                     : freq == freq_quarterly_apr  ? "quarterly_apr"
                                     : freq == freq_quarterly_may  ? "quarterly_may"
                                     : freq == freq_quarterly_jun  ? "quarterly_jun"
                                     : freq == freq_quarterly_jul  ? "quarterly_jul"
                                     : freq == freq_quarterly_aug  ? "quarterly_aug"
                                     : freq == freq_quarterly_sep  ? "quarterly_sep"
                                     : freq == freq_quarterly_oct  ? "quarterly_oct"
                                     : freq == freq_quarterly_nov  ? "quarterly_nov"
                                     : freq == freq_quarterly_dec  ? "quarterly_dec"
                                     : freq == freq_halfyearly     ? "halfyearly"
                                     : freq == freq_halfyearly_jan ? "halfyearly_jan"
                                     : freq == freq_halfyearly_feb ? "halfyearly_feb"
                                     : freq == freq_halfyearly_mar ? "halfyearly_mar"
                                     : freq == freq_halfyearly_apr ? "halfyearly_apr"
                                     : freq == freq_halfyearly_may ? "halfyearly_may"
                                     : freq == freq_halfyearly_jun ? "halfyearly_jun"
                                     : freq == freq_halfyearly_jul ? "halfyearly_jul"
                                     : freq == freq_halfyearly_aug ? "halfyearly_aug"
                                     : freq == freq_halfyearly_sep ? "halfyearly_sep"
                                     : freq == freq_halfyearly_oct ? "halfyearly_oct"
                                     : freq == freq_halfyearly_nov ? "halfyearly_nov"
                                     : freq == freq_halfyearly_dec ? "halfyearly_dec"
                                     : freq == freq_yearly         ? "yearly"
                                     : freq == freq_yearly_jan     ? "yearly_jan"
                                     : freq == freq_yearly_feb     ? "yearly_feb"
                                     : freq == freq_yearly_mar     ? "yearly_mar"
                                     : freq == freq_yearly_apr     ? "yearly_apr"
                                     : freq == freq_yearly_may     ? "yearly_may"
                                     : freq == freq_yearly_jun     ? "yearly_jun"
                                     : freq == freq_yearly_jul     ? "yearly_jul"
                                     : freq == freq_yearly_aug     ? "yearly_aug"
                                     : freq == freq_yearly_sep     ? "yearly_sep"
                                     : freq == freq_yearly_oct     ? "yearly_oct"
                                     : freq == freq_yearly_nov     ? "yearly_nov"
                                     : freq == freq_yearly_dec     ? "yearly_dec"
                                                                   : "unknown");
    return out << '"';
}

int print_value(type_t obj_type, int64_t nbytes, const void *value, int i)
{
    switch (obj_type)
    {
    case type_integer:
    {
        if (nbytes == 1)
            D << reinterpret_cast<const int8_t *>(value)[i];
        else if (nbytes == 2)
            D << reinterpret_cast<const int16_t *>(value)[i];
        else if (nbytes == 4)
            D << reinterpret_cast<const int32_t *>(value)[i];
        else if (nbytes == 8)
            D << reinterpret_cast<const int64_t *>(value)[i];
        else
        {
            fprintf(stderr, "type_integer with unexpected nbytes = %ld", nbytes);
            D << reinterpret_cast<const int64_t *>(value)[i];
        }
        break;
    }
    case type_unsigned:
    {
        if (nbytes == 1)
            D << reinterpret_cast<const uint8_t *>(value)[i];
        else if (nbytes == 2)
            D << reinterpret_cast<const uint16_t *>(value)[i];
        else if (nbytes == 4)
            D << reinterpret_cast<const uint32_t *>(value)[i];
        else if (nbytes == 8)
            D << reinterpret_cast<const uint64_t *>(value)[i];
        else
        {
            fprintf(stderr, "type_unsigned with unexpected nbytes = %ld", nbytes);
            D << reinterpret_cast<const uint64_t *>(value)[i];
        }
        break;
    }
    case type_float:
    {
        D << std::setprecision(15);
        if (nbytes == 4)
            D << reinterpret_cast<const float *>(value)[i];
        else if (nbytes == 8)
            D << reinterpret_cast<const double *>(value)[i];
        else
        {
            fprintf(stderr, "type_float with unexpected nbytes = %ld", nbytes);
            D << reinterpret_cast<const long double *>(value)[i];
        }
        break;
    }
    case type_string:
    {
        if (i != 0)
        {
            fprintf(stderr, "Can't print series of string yet");
            return -1;
        }
        std::string s(reinterpret_cast<const char *>(value));
        D << '"' << s << '"';
        break;
    }
    default:
    {
        fprintf(stderr, "can't print value of scalar type %d", obj_type);
    }
    }
    return 0;
}

int export_scalar(const object_t &obj)
{
    scalar_t scal;
    int rc = de_load_scalar(de, obj.id, &scal);
    if (rc != DE_SUCCESS)
    {
        return print_de_error();
    }

    const char *oname;
    if (obj.pid == 0)
    {
        oname = obj.name;
    }
    else
    {
        rc = de_get_object_info(de, obj.id, &oname, NULL, NULL);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
            return rc;
        }
    }

    std::string name(oname);

    if (M.is_open())
    {
        M << '"' << name << '"' << ','
          << obj.obj_class << ','
          << obj.obj_type << ','
          << scal.frequency << '\n';
    }
    if (D.is_open())
    {
        D << '"' << "N/A" << '"' << ',' << '"' << name << '"' << ',';
        print_value(scal.object.obj_type, scal.nbytes, scal.value, 0);
        D << '\n';
    }
    return 0;
}

int print_date(frequency_t freq, date_t date)
{
    int32_t year;
    uint32_t month, day;
    if (de_unpack_calendar_date(freq, date, &year, &month, &day) != DE_SUCCESS)
    {
        return print_de_error();
    }
    D << year << '-' << month << '-' << day;
    return 0;
}

int print_series_row(const tseries_t &ser, const std::string &name)
{
    frequency_t freq = ser.axis.frequency;
    date_t date = ser.axis.first;
    int64_t nbytes = ser.nbytes / ser.axis.length;
    for (auto i = 0; i < ser.axis.length; ++i)
    {
        D << '"';
        if (print_date(freq, date + i) != 0)
        {
            return -1;
        };
        D << '"' << ',' << '"' << name << '"' << ',';
        if (print_value(ser.eltype, nbytes, ser.value, i) != 0)
        {
            return -1;
        };
        D << '\n';
    }
    return 0;
}

int export_series(const object_t &obj)
{
    tseries_t ser;
    int rc = de_load_tseries(de, obj.id, &ser);
    if (rc != DE_SUCCESS)
    {
        return print_de_error();
    }

    const char *oname;
    if (obj.pid == 0)
    {
        oname = obj.name;
    }
    else
    {
        rc = de_get_object_info(de, obj.id, &oname, NULL, NULL);
        if (rc != DE_SUCCESS)
        {
            print_de_error();
            return rc;
        }
    }

    std::string name(oname);

    if (M.is_open())
    {
        M << '"' << name << '"' << ','
          << obj.obj_class << ','
          << obj.obj_type << ','
          << ser.axis.frequency << '\n';
    }
    if (D.is_open())
    {
        return print_series_row(ser, oname);
    }
    return 0;
}

int export_catalog(obj_id_t pid)
{
    int rc;
    object_t obj;
    de_search search;
    if (export_scalars)
    {
        if (de_search_catalog(de, pid, NULL, type_any, class_scalar, &search) != DE_SUCCESS)
        {
            return print_de_error();
        }
        rc = de_next_object(search, &obj);
        while (rc == DE_SUCCESS)
        {
            rc = export_scalar(obj);
            if (rc != 0)
                return rc;
            rc = de_next_object(search, &obj);
        }
        if (rc != DE_NO_OBJ)
        {
            return print_de_error();
        }
    }
    /* now repeat for tseries */
    if (de_search_catalog(de, pid, NULL, type_any, class_tseries, &search) != DE_SUCCESS)
    {
        return print_de_error();
    }
    rc = de_next_object(search, &obj);
    while (rc == DE_SUCCESS)
    {
        rc = export_series(obj);
        if (rc != 0)
            return rc;
        rc = de_next_object(search, &obj);
    }
    if (rc != DE_NO_OBJ)
    {
        return print_de_error();
    }
    /* now do any catalogs recursively */
    if (de_search_catalog(de, pid, NULL, type_any, class_catalog, &search) != DE_SUCCESS)
    {
        return print_de_error();
    }
    rc = de_next_object(search, &obj);
    while (rc == DE_SUCCESS)
    {
        rc = export_catalog(obj.id);
        if (rc != 0)
            return rc;
        rc = de_next_object(search, &obj);
    }
    if (rc != DE_NO_OBJ)
    {
        return print_de_error();
    }
    return 0;
}

void close_all()
{
    if (M.is_open())
        M.close();
    if (D.is_open())
        D.close();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (de_open_readonly(argv[1], &de) != DE_SUCCESS)
    {
        return print_de_error();
    }

    if (argc >= 2)
    {
        D.open(argv[2], std::ios::out | std::ios::trunc);
        if (argc > 2)
        {
            M.open(argv[3], std::ios::out | std::ios::trunc);
        }
    }

    print_header();

    int rc = export_catalog(0);

    if (D.is_open())
        D.close();
    if (M.is_open())
        M.close();

    return rc;
}

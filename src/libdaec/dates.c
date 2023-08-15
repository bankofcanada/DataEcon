
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "dates.h"

/* https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.3172 */

/* internal calculations are done with epoch such that 0000-03-01 => 0 */

/* we want user to see external rata die numbers such that 0001-01-01 => 1, which is the
   same same as Julia's standard library Dates.Date) */

/* Set this constant equal to the internal rata die of the date we want to have external rata die 0*/
#define EPOCH_ZERO_DAY 305

/* Set this constant equal to the internal rata die of the monday of the week we want to have external rata septem 0 */
#define EPOCH_ZERO_MONDAY 299

/* if we want the unix epoch, that is 1979-01-01 => 0 */
// #define EPOCH_ZERO_DAY 719468
// #define EPOCH_ZERO_MONDAY 719465

/* If we want the internal epoch, that is 0000-03-01 => 0 (calculations are actually done with this epoch) */
// #define EPOCH_ZERO_DAY 0
// #define EPOCH_ZERO_MONDAY -2

/* Shift and correction constants. */
static const uint32_t EPOCH_s = 82;
static const uint32_t EPOCH_K = 146097 * EPOCH_s;
static const uint32_t EPOCH_L = 400 * EPOCH_s;

static const uint32_t EPOCH_Kw = 20871 * EPOCH_s;
static const uint32_t EPOCH_Kb = 104355 * EPOCH_s;
static const uint32_t EPOCH_ZERO_BDAY_SHIFT = (EPOCH_ZERO_DAY - EPOCH_ZERO_MONDAY) > 4 ? 4 : (EPOCH_ZERO_DAY - EPOCH_ZERO_MONDAY);

struct __internal_date
{
    int32_t year;
    uint32_t month;
    uint32_t day;
};

/*
    Convert a rata die number to a proleptic Gregorian date.
    Epoch offset given in EPOCH_DAYS
*/
struct __internal_date _rata_die_to_date(int32_t N_U)
{
    /* Figure 12 in https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.3172 */

    /* Shift rata die from epoch to standard */
    const uint32_t N = N_U + EPOCH_K + EPOCH_ZERO_DAY;

    /* Century */
    const uint32_t N_1 = 4 * N + 3;
    const uint32_t C = N_1 / 146097;
    const uint32_t N_C = N_1 % 146097 / 4;

    /* Year */
    const uint32_t N_2 = 4 * N_C + 3;
    const uint64_t P_2 = ((uint64_t)2939745) * N_2;
    const uint32_t Z = (uint32_t)(P_2 / 4294967296);
    const uint32_t N_Y = (uint32_t)(P_2 % 4294967296 / 2939745 / 4);
    const uint32_t Y = 100 * C + Z;

    /* Month and Day */
    const uint32_t N_3 = 2141 * N_Y + 197913;
    const uint32_t M = N_3 / 65536;
    const uint32_t D = N_3 % 65536 / 2141;

    /* Map from computational date to proleptic Gregorian date with epoch shift */
    const uint32_t J = N_Y >= 306;
    const int32_t Y_G = (Y - EPOCH_L) + J;
    const uint32_t M_G = J ? M - 12 : M;
    const uint32_t D_G = D + 1;

    struct __internal_date ret = {.year = Y_G, .month = M_G, .day = D_G};
    return ret;
}

/*
    Convert a proleptic Gregorian date to its rata die number.
    Epoch offset given in EPOCH_DAYS
*/
int32_t _date_to_rata_die(struct __internal_date date)
{
    /* Figure 13 in https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.3172 */

    /* Convert proleptic Gregorian date to computational date */
    const uint32_t J = date.month <= 2;
    const uint32_t Y = (((uint32_t)date.year) + EPOCH_L) - J;
    const uint32_t M = J ? date.month + 12 : date.month;
    const uint32_t D = date.day - 1;
    const uint32_t C = Y / 100;

    /* Calculate rata die of computational date */
    const uint32_t y_star = 1461 * Y / 4 - C + C / 4;
    const uint32_t m_star = (979 * M - 2919) / 32;
    const uint32_t N = y_star + m_star + D;

    /* Shift rata die to match epoch */
    return N - EPOCH_K - EPOCH_ZERO_DAY;
}

/* weekly frequency mon-sun */
int32_t _rata_die_to_septem(int32_t N_U)
{
    const uint32_t N = N_U + EPOCH_K + EPOCH_ZERO_DAY;
    const uint32_t Nw = (N - EPOCH_ZERO_MONDAY) / 7;
    const int32_t Nw_U = Nw - EPOCH_Kw;
    return Nw_U;
}

/* convert day number to business-day number */
/* inexact, if not NULL, returns 0 - all good, 1 - Saturday, 2 - Sunday */
int32_t _rata_die_to_profesto(int32_t N_U, int32_t *inexact)
{
    const uint32_t N = N_U + EPOCH_K + EPOCH_ZERO_DAY;
    const uint32_t N_1 = N - EPOCH_ZERO_MONDAY;
    const uint32_t Nw = N_1 / 7;
    const uint32_t O = N_1 % 7;
    const uint32_t J = O <= 4;
    if (inexact)
    {
        *inexact = J ? 0 : O - 4;
    }
    const uint32_t G = J ? O : 4;
    const uint32_t Nb = 5 * Nw + G;
    const int32_t Nb_U = Nb - EPOCH_Kb - EPOCH_ZERO_BDAY_SHIFT;
    return Nb_U;
}

int32_t _rata_die_from_septem(int32_t Nw_U)
{
    const uint32_t Nw = Nw_U + EPOCH_Kw;
    const uint32_t N = 7 * Nw + EPOCH_ZERO_MONDAY;
    const int32_t N_U = N - EPOCH_K - EPOCH_ZERO_DAY;
    return N_U;
}

int32_t _rata_die_from_profesto(int32_t Nb_U)
{
    const uint32_t Nb = Nb_U + EPOCH_Kb + EPOCH_ZERO_BDAY_SHIFT;
    const uint32_t Nw = Nb / 5;
    const uint32_t G = Nb % 5;
    const uint32_t N = 7 * Nw + G + EPOCH_ZERO_MONDAY;
    const int32_t N_U = N - EPOCH_K - EPOCH_ZERO_DAY;
    return N_U;
}

/*****************************************************************************************/

static const int YP_FREQS = freq_monthly | freq_quarterly | freq_halfyearly | freq_yearly;

int de_pack_date(frequency_t freq, int64_t value, date_t *date)
{
    if (date == NULL)
        return error(DE_NULL);
    date->freq = freq;
    date->value = value;
    return DE_SUCCESS;
}

bool _has_ppy(frequency_t freq)
{
    return (freq & YP_FREQS) ? true : false;
}

int32_t _get_ppy(frequency_t freq)
{
    switch (freq & YP_FREQS)
    {
    case freq_monthly:
        return 12;
    case freq_quarterly:
        return 4;
    case freq_halfyearly:
        return 2;
    case freq_yearly:
        return 1;
    case 0:
        return 0;
    default:
        return -1;
    }
}

int de_pack_year_period_date(frequency_t freq, int year, int period, date_t *date)
{
    if (date == NULL)
        return error(DE_NULL);
    int32_t N;
    if (_has_ppy(freq))
    {
        N = year * _get_ppy(freq);
    }
    else
    {
        struct __internal_date _idate = {.year = year, .month = 1, .day = 1};
        N = _date_to_rata_die(_idate);
        if (freq == freq_daily)
        {
            /* nothing */
        }
        else if (freq == freq_bdaily)
        {
            /* something */
            int32_t inexact;
            N = _rata_die_to_profesto(N, &inexact);
            if (inexact)
                return error(DE_INEXACT);            
        }
        else if (freq & freq_weekly)
        {
            uint32_t offset = (freq ^ freq_weekly) % 7;
            if (offset != 0)
                return error(DE_INTERNAL);
            N = _rata_die_to_septem(N);
        }
        else
        {
            return error(DE_INTERNAL);
        }
    }
    TRACE_RUN(de_pack_date(freq, N + period - 1, date));
    return DE_SUCCESS;
}

int de_pack_calendar_date(frequency_t freq, int year, int month, int day, date_t *date)
{
    if (date == NULL)
        return error(DE_NULL);
    if ((freq == freq_daily) || (freq == freq_bdaily) || (freq && freq_weekly))
    {
        struct __internal_date _idate = {.year = year, .month = month, .day = day};
        TRACE_RUN(de_pack_date(freq, _date_to_rata_die(_idate), date));
        return DE_SUCCESS;
    }
    else
        return error(DE_BAD_FREQ);
}

int de_unpack_date(date_t date, frequency_t *freq, int64_t *value)
{
    if (freq == NULL || value == NULL)
        return error(DE_NULL);
    *freq = date.freq;
    *value = date.value;
    return DE_SUCCESS;
}

int de_unpack_year_period_date(date_t date, frequency_t *freq, int *year, int *period)
{
    if (freq == NULL || year == NULL || period == NULL)
        return error(DE_NULL);
    int64_t value;
    TRACE_RUN(de_unpack_date(date, freq, &value));
    int ppy = _get_ppy(*freq);
    if (ppy == 0)
        return error(DE_BAD_FREQ);
    if (ppy < 0)
        return error(DE_INTERNAL);
    div_t dv = div(value, ppy);
    if (dv.rem < 0)
    {
        *year = dv.quot - 1;
        *period = dv.rem + ppy + 1;
    }
    else
    {
        *year = dv.quot;
        *period = dv.rem + 1;
    }
    return DE_SUCCESS;
}

int de_unpack_calendar_date(date_t date, frequency_t *freq, int *year, int *month, int *day)
{
    if (freq == NULL || year == NULL || month == NULL || day == NULL)
        return error(DE_NULL);
    int64_t value;
    TRACE_RUN(de_unpack_date(date, freq, &value));
    if ((*freq == freq_daily) || (*freq == freq_bdaily) || (*freq && freq_weekly))
    {
        struct __internal_date _idate = _rata_die_to_date(value);
        *year = _idate.year;
        *month = _idate.month;
        *day = _idate.day;
        return DE_SUCCESS;
    }
    else
        return error(DE_BAD_FREQ);
}


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.h"
#include "file.h"
#include "object.h"
#include "dates.h"

/* https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.3172 */

/* gives 0001-01-01 => 1  (same as Julia standard library Dates.Date) */
#define EPOCH_DAYS 305

/* gives 1979-01-01 => 0  (number of days since UNIX Epoch ) */
// #define EPOCH_DAYS 719468

/* gives 0000-03-01 => 0  (standard computational date epoch) */
// #define EPOCH_DAYS 0

/* gives 0000-01-01 => 0  (standard computational date epoch) */
// #define EPOCH_DAYS -59

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

    /* Shift and correction constants. */
    static const uint32_t s = 82;
    static const uint32_t K = EPOCH_DAYS + 146097 * s;
    static const uint32_t L = 400 * s;

    /* Shift rata die from epoch to standard */
    const uint32_t N = N_U + K;

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
    const int32_t Y_G = (Y - L) + J;
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

    /* Shift and correction constants. */
    static const uint32_t s = 82;
    static const uint32_t K = EPOCH_DAYS + 146097 * s;
    static const uint32_t L = 400 * s;

    /* Convert proleptic Gregorian date to computational date */
    const uint32_t J = date.month <= 2;
    const uint32_t Y = (((uint32_t)date.year) + L) - J;
    const uint32_t M = J ? date.month + 12 : date.month;
    const uint32_t D = date.day - 1;
    const uint32_t C = Y / 100;

    /* Calculate rata die of computational date */
    const uint32_t y_star = 1461 * Y / 4 - C + C / 4;
    const uint32_t m_star = (979 * M - 2919) / 32;
    const uint32_t N = y_star + m_star + D;

    /* Shift rata die to match epoch */
    return N - K;
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

int _get_ppy(frequency_t freq)
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
    int ppy = _get_ppy(freq);
    if (ppy > 0)
    {
        TRACE_RUN(de_pack_date(freq, year * ppy + period - 1, date));
        return DE_SUCCESS;
    }
    else if (ppy == 0)
        return error(DE_BAD_FREQ);
    else
        return error(DE_INTERNAL);
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

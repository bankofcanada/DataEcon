
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

/* weekly frequency; eow = last day of the week mon=1, sun=7 */
int32_t _rata_die_to_septem(int32_t N_U, uint32_t eow)
{
    const uint32_t O_1 = eow % 7;
    const uint32_t O = (O_1 == 0) ? 0 : (7 - O_1);
    const uint32_t N = N_U + EPOCH_K + EPOCH_ZERO_DAY;
    const uint32_t Nw = (N - EPOCH_ZERO_MONDAY + O) / 7;
    const int32_t Nw_U = Nw - EPOCH_Kw;
    return Nw_U;
}

/* convert day number to business-day number */
/* weekend, if not NULL, returns 0 - weekday, 1 - Saturday, 2 - Sunday */
int32_t _rata_die_to_profesto(int32_t N_U, uint32_t *weekend)
{
    const uint32_t N = N_U + EPOCH_K + EPOCH_ZERO_DAY;
    const uint32_t N_1 = N - EPOCH_ZERO_MONDAY;
    const uint32_t Nw = N_1 / 7;
    const uint32_t O = N_1 % 7;
    const uint32_t J = O <= 4;
    const uint32_t G = J ? O : 4;
    const uint32_t Nb = 5 * Nw + G;
    const int32_t Nb_U = Nb - EPOCH_Kb - EPOCH_ZERO_BDAY_SHIFT;
    if (weekend)
    {
        *weekend = J ? 0 : O - 4;
    }
    return Nb_U;
}

int32_t _rata_die_from_septem(int32_t Nw_U, uint32_t eow)
{
    const uint32_t O_1 = eow % 7;
    const uint32_t O = (O_1 == 0) ? 0 : (7 - O_1);
    const uint32_t Nw = Nw_U + EPOCH_Kw;
    const uint32_t N = 7 * Nw + EPOCH_ZERO_MONDAY;
    const int32_t N_U = N + 6 - O - EPOCH_K - EPOCH_ZERO_DAY;
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
/* encoding and decoding dates with ppy frequencies */

static const int YP_FREQS = freq_monthly | freq_quarterly | freq_halfyearly | freq_yearly;

bool _has_ppy(frequency_t freq)
{
    return (freq & YP_FREQS) ? true : false;
}

int _get_ppy(frequency_t freq, uint32_t *ppy)
{
    switch (freq & YP_FREQS)
    {
    case freq_monthly:
        *ppy = 12;
        return DE_SUCCESS;
    case freq_quarterly:
        *ppy = 4;
        return DE_SUCCESS;
    case freq_halfyearly:
        *ppy = 2;
        return DE_SUCCESS;
    case freq_yearly:
        *ppy = 1;
        return DE_SUCCESS;
    }
    /* not a ppy frequency */
    return error(DE_INTERNAL);
}

int _encode_ppy(frequency_t freq, int32_t year, uint32_t period, int32_t *N)
{
    uint32_t ppy;
    TRACE_RUN(_get_ppy(freq, &ppy));
    *N = year * ppy + period - 1;
    return DE_SUCCESS;
}

int _decode_ppy(frequency_t freq, int32_t N_U, int32_t *year, uint32_t *period)
{
    uint32_t ppy;
    TRACE_RUN(_get_ppy(freq, &ppy));
    const uint32_t N = N_U + EPOCH_L * ppy;
    *period = N % ppy + 1;
    *year = N / ppy - EPOCH_L;
    return DE_SUCCESS;
}

/*****************************************************************************************/
/* encoding and decoding dates with calendar frequencies */

int _encode_calendar(frequency_t freq, int32_t year, uint32_t month, uint32_t day, int32_t *N)
{
    /* the formulas we use are guaranteed to work for signed 16-bit year */
    /* the formulas work for 0 <= month <= 14 and is known to fail for month = 15 or more.
        month = 0 is December of the previous year,
        month=13 or 14 is Jan or Feb of the following year */
    if (year < -32800 || year > 32800 || month > 14)
        return error(DE_RANGE);
    const struct __internal_date _idate = {.year = year, .month = month, .day = day};
    *N = _date_to_rata_die(_idate);
    if (freq == freq_daily)
        return DE_SUCCESS;
    if (freq == freq_bdaily)
    {
        uint32_t weekend;
        *N = _rata_die_to_profesto(*N, &weekend);
        if (weekend > 0)
            return error(DE_INEXACT);
        else
            return DE_SUCCESS;
    }
    if (freq & freq_weekly)
    {
        *N = _rata_die_to_septem(*N, freq % freq_weekly);
        return DE_SUCCESS;
    }
    return error(DE_INTERNAL);
}

int _encode_first_period(frequency_t freq, int32_t year, int32_t *N)
{
    int rc = _encode_calendar(freq, year, 1, 1, N);
    if (rc == DE_INEXACT)
    {
        /* freq == freq_bdaily and Jan 1-st is on a weekend */
        *N = *N + 1;
        rc = de_clear_error();
    }
    return rc;
}

int _decode_calendar(frequency_t freq, int32_t N, int32_t *year, uint32_t *month, uint32_t *day)
{
    if (freq == freq_daily)
    {
        // N = N;
    }
    else if (freq == freq_bdaily)
    {
        N = _rata_die_from_profesto(N);
    }
    else if (freq & freq_weekly)
    {
        N = _rata_die_from_septem(N, freq % freq_weekly);
    }
    else
        return error(DE_INTERNAL);
    const struct __internal_date d = _rata_die_to_date(N);
    *year = d.year;
    *month = d.month;
    *day = d.day;
    return DE_SUCCESS;
}

/*****************************************************************************************/
/* pack and unpack dates given by year and period */

int de_pack_year_period_date(frequency_t freq, int32_t year, uint32_t period, date_t *date)
{
    if (date == NULL)
        return error(DE_NULL);
    int32_t N;
    if (_has_ppy(freq))
    {
        TRACE_RUN(_encode_ppy(freq, year, period, &N));
    }
    else
    {
        TRACE_RUN(_encode_first_period(freq, year, &N));
        N = N + period - 1;
    }
    *date = N;
    return DE_SUCCESS;
}

int de_unpack_year_period_date(frequency_t freq, date_t date, int32_t *year, uint32_t *period)
{
    if (year == NULL || period == NULL)
        return error(DE_NULL);
    const int32_t N = date;
    if (_has_ppy(freq))
    {
        TRACE_RUN(_decode_ppy(freq, N, year, period));
    }
    else
    {
        int32_t NY;
        uint32_t M, D;
        /* call decode to get the year */
        TRACE_RUN(_decode_calendar(freq, N, year, &M, &D));
        /* call encode to get the N-value of first period of the year */
        TRACE_RUN(_encode_first_period(freq, *year, &NY));
        *period = N - NY + 1;
    }
    return DE_SUCCESS;
}

/*****************************************************************************************/
/* pack and unpack dates given by year, month and day */

int de_pack_calendar_date(frequency_t freq, int32_t year, uint32_t month, uint32_t day, date_t *date)
{
    if (date == NULL)
        return error(DE_NULL);
    int32_t N;
    if (_has_ppy(freq))
    {
        /*
        uint32_t ppy;
        TRACE_RUN(_get_ppy(freq, &ppy));
        if (ppy > 12)
            return error(DE_INTERNAL);
        const uint32_t P = (month - 1) * ppy / 12 + 1;
        const uint32_t R = (month - 1) % ppy;
        if (R)
        TRACE_RUN(_encode_ppy(freq, year, P, &N));
        */

        // This case is not implemented -- it requires frequency conversions
        return error(DE_INTERNAL);
    }
    else
    {
        TRACE_RUN(_encode_calendar(freq, year, month, day, &N));
    }
    *date = N;
    return DE_SUCCESS;
}

int de_unpack_calendar_date(frequency_t freq, date_t date, int32_t *year, uint32_t *month, uint32_t *day)
{
    if (year == NULL || month == NULL || day == NULL)
        return error(DE_NULL);
    const int32_t N = date;
    if (_has_ppy(freq))
    {
        // This case is not implemented -- it requires frequency conversions
        return error(DE_INTERNAL);
    }
    else
    {
        TRACE_RUN(_decode_calendar(freq, N, year, month, day));
    }
    return DE_SUCCESS;
}
#ifndef __DATES_H__
#define __DATES_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "file.h"
#include "object.h"

/*

    Dates are used to index into tseries and mvtseries, so they are integers.
    Dates also have a frequency information. The encoding must be such that
    adding 1 gives the next date in the given frequency

    We have several kinds of frequencies
    - no frequency - that should mean an error, but not necessarily, depending on the situation
    - unit frequency - indicates that we're using a regular integer instead of a date
    - calendar frequency - includes daily, weekly and business-daily
    - year-period frequency - monthly, quarterly, half-yearly, yearly
    - custom

    ## Year-period
    The frequency is represented with number of periods per year (ppy)
    A date with year-period frequency contains year and period. The date is
    encoded as year * ppy + period

    ## Calendar
    The daily date is encoded as the number of days since a reference date.
    The encoding for weekly is the daily encoding divided by 7, with
    a the remainder according to the end date of the frequency.

*/

typedef enum
{
    freq_none = 0,
    freq_unit = 11,
    freq_daily = 12,
    freq_bdaily = 13,
    freq_weekly = 16,
    freq_weekly_sun0 = freq_weekly,
    freq_weekly_mon,
    freq_weekly_tue,
    freq_weekly_wed,
    freq_weekly_thu,
    freq_weekly_fri,
    freq_weekly_sat,
    freq_weekly_sun7,
    freq_weekly_sun = freq_weekly_sun7,
    freq_monthly = 32,
    freq_quarterly = 64,
    freq_quarterly_jan = freq_quarterly + 1,
    freq_quarterly_feb,
    freq_quarterly_mar,
    freq_quarterly_apr = freq_quarterly + 1,
    freq_quarterly_may,
    freq_quarterly_jun,
    freq_quarterly_jul = freq_quarterly + 1,
    freq_quarterly_aug,
    freq_quarterly_sep,
    freq_quarterly_oct = freq_quarterly + 1,
    freq_quarterly_nov,
    freq_quarterly_dec,
    freq_halfyearly = 128,
    freq_halfyearly_jan = freq_halfyearly + 1,
    freq_halfyearly_feb,
    freq_halfyearly_mar,
    freq_halfyearly_apr,
    freq_halfyearly_may,
    freq_halfyearly_jun,
    freq_halfyearly_jul = freq_halfyearly + 1,
    freq_halfyearly_aug,
    freq_halfyearly_sep,
    freq_halfyearly_oct,
    freq_halfyearly_nov,
    freq_halfyearly_dec,
    freq_yearly = 256,
    freq_yearly_jan = freq_yearly + 1,
    freq_yearly_feb,
    freq_yearly_mar,
    freq_yearly_apr,
    freq_yearly_may,
    freq_yearly_jun,
    freq_yearly_jul,
    freq_yearly_aug,
    freq_yearly_sep,
    freq_yearly_oct,
    freq_yearly_nov,
    freq_yearly_dec,
} frequency_t;

typedef int64_t date_t;

int de_pack_year_period_date(frequency_t freq, int32_t year, uint32_t period, date_t *date);
int de_unpack_year_period_date(frequency_t freq, date_t date, int32_t *year, uint32_t *period);

int de_pack_calendar_date(frequency_t freq, int32_t year, uint32_t month, uint32_t day, date_t *date);
int de_unpack_calendar_date(frequency_t freq, date_t date, int32_t *year, uint32_t *month, uint32_t *day);

#endif

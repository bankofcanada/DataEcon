
#include "daec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>

struct __internal_date
{
    int32_t year;
    uint32_t month;
    uint32_t day;
};
extern struct __internal_date _rata_die_to_date(int32_t N_U);
extern int32_t _date_to_rata_die(struct __internal_date date);
int32_t _rata_die_to_profesto(int32_t N_U, int32_t *inexact);
int32_t _rata_die_to_septem(int32_t N_U, uint32_t eow);
int32_t _rata_die_from_profesto(int32_t Nw_U);
int32_t _rata_die_from_septem(int32_t Nw_U, uint32_t eow);

int main(int argc, char **argv)
{
    printf("Welcome to the playground!\n");

    while (true)
    {
        struct __internal_date d;
        int32_t n, nw, nb, inexact;
        printf("Enter a date: YYYY-MM-DD: ");
        scanf("%d-%u-%u", &d.year, &d.month, &d.day);
        n = _date_to_rata_die(d);
        nw = _rata_die_to_septem(n, 0);
        printf("       From N:  N = %-10d  %d-%d-%d\n", n, d.year, d.month, d.day);
        nb = _rata_die_to_profesto(n, &inexact);
        printf("    N = %-10d Nw = %-10d Nb = %-10d inexact = %d\n", n, nw, nb, inexact);
        d = _rata_die_to_date(n);
        printf("       From N:  N = %-10d  %d-%d-%d\n", n, d.year, d.month, d.day);
        int32_t n_1 = _rata_die_from_septem(nw, 0);
        d = _rata_die_to_date(n_1);
        printf("       From Nw: N = %-10d  %d-%d-%d\n", n_1, d.year, d.month, d.day);
        int32_t n_2 = _rata_die_from_profesto(nb);
        d = _rata_die_to_date(n_2);
        printf("       From Nb: N = %-10d  %d-%d-%d\n", n_2, d.year, d.month, d.day);
        for (uint32_t i = 0; i <= 7; ++i)
        {
            nw = _rata_die_to_septem(n, i);
            n_1 = _rata_die_from_septem(nw, i);
            d = _rata_die_to_date(n_1);
            printf("       Nw(%i) = %d => N = %d (%d-%d-%d)\n", i, nw, n_1, d.year, d.month, d.day);
        }
    }

    return EXIT_SUCCESS;
}

// Prove src/small_divide.h matches the native / operator, exhaustively
//
// Copyright (C) 2026 Pono Data Solutions
//
// This file may be distributed under the terms of the GNU GPLv3 license.
//
// The MCU targets that need small_divide.h have no divide instruction, so the
// arithmetic there cannot be spot-checked against a hardware divide. Run this
// on the build host instead, where / is exact by definition:
//
//     cc -O2 -Isrc -o /tmp/verify-small-divide scripts/verify-small-divide.c
//     /tmp/verify-small-divide
//
// It walks every one of the 2^32 unsigned and 2^32 signed dividends against
// every divisor in range, so a passing run is a proof and not a sample. Takes
// a few minutes. Re-run it after any edit to the table or to the generator.

#include <stdint.h>
#include <stdio.h>

#include "small_divide.h"

int
main(void)
{
    long long total_bad = 0;

    for (uint32_t d = 1; d <= SMALL_DIVIDE_MAX; d++) {
        long long bad = 0;
        uint32_t n = 0;
        do {
            if (udiv_small(n, d) != n / d && bad++ < 5)
                printf("  udiv_small(%u, %u) = %u, expected %u\n",
                       n, d, udiv_small(n, d), n / d);
            n++;
        } while (n != 0);

        int32_t s = INT32_MIN;
        for (;;) {
            if (sdiv_small(s, d) != s / (int32_t)d && bad++ < 5)
                printf("  sdiv_small(%d, %u) = %d, expected %d\n",
                       s, d, sdiv_small(s, d), s / (int32_t)d);
            if (s == INT32_MAX)
                break;
            s++;
        }

        printf("  d=%2u  %s\n", d, bad ? "MISMATCH" : "exact, all 2^33 cases");
        fflush(stdout);
        total_bad += bad;
    }

    // sdiv_10000 carries its own magic, so it needs its own sweep.
    long long bad = 0;
    int32_t s = INT32_MIN;
    for (;;) {
        if (sdiv_10000(s) != s / 10000 && bad++ < 5)
            printf("  sdiv_10000(%d) = %d, expected %d\n",
                   s, sdiv_10000(s), s / 10000);
        if (s == INT32_MAX)
            break;
        s++;
    }
    printf("  d=10000  %s\n", bad ? "MISMATCH" : "exact, all 2^32 cases");
    total_bad += bad;

    printf("%s\n", total_bad ? "FAILED" : "PASS: every divisor exact");
    return total_bad ? 1 : 0;
}

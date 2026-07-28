#!/usr/bin/env python3
# Generate the reciprocal table in src/small_divide.h
#
# Copyright (C) 2026 Pono Data Solutions
#
# This file may be distributed under the terms of the GNU GPLv3 license.
"""Emit the magic/add/shift triples used by src/small_divide.h.

Standard round-up reciprocal (Hacker's Delight, magicu). For each divisor it
finds the smallest p >= 32 with

    2^p > nc * (d - 1 - (2^p - 1) mod d),   nc = floor(2^32 / d) * d - 1

which is the condition under which floor(n * ceil(2^p / d) / 2^p) equals
floor(n / d) for every 32-bit n. When the resulting magic needs 33 bits the
top bit is dropped and the `add` flag tells the C side to fold the carry back
in, which costs one shift and one add and keeps every intermediate in 32 bits.

Print the table and paste it into src/small_divide.h. Nothing here is trusted
on the strength of the derivation: scripts/verify-small-divide.c compares the
result against the native operators for all 2^32 unsigned and all 2^32 signed
dividends, on every divisor.
"""
import sys

MAXD = 16


def magicu(d):
    nc = ((1 << 32) // d) * d - 1
    p = 31
    while True:
        p += 1
        if p > 64:
            raise SystemExit("no 32-bit magic for %d" % d)
        if (1 << p) > nc * (d - 1 - ((1 << p) - 1) % d):
            break
    m = ((1 << p) + d - 1 - ((1 << p) - 1) % d) // d
    add = 0
    s = p - 32
    if m >= (1 << 32):
        m -= (1 << 32)
        add = 1
        # the carry fold halves the remaining shift
        s -= 1
    return m, add, s


def main():
    extra = [int(a) for a in sys.argv[1:]]

    print("static const struct small_divide_entry "
          "small_divide_tab[SMALL_DIVIDE_MAX + 1] = {")
    print("    {          0u, 0,  0 },  //  0  unused")
    for d in range(1, MAXD + 1):
        if (d & (d - 1)) == 0:                      # power of two: plain shift
            print("    {          0u, 0, %2d },  // %2d  shift"
                  % (d.bit_length() - 1, d))
            continue
        m, add, s = magicu(d)
        print("    { %10uu, %d, %2d },  // %2d%s"
              % (m, add, s, d, "  add form" if add else ""))
    print("};")

    for d in extra:
        m, add, s = magicu(d)
        print("\n// divisor %d: m=%u add=%d s=%d" % (d, m, add, s))
        print("//   q = (uint32_t)(((uint64_t)u * %uu) >> 32)%s >> %d;"
              % (m, "  [+ carry fold]" if add else "", s))


if __name__ == "__main__":
    main()

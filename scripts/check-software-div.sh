#!/bin/bash
# Check if a binary appears to have a software library divide operator

CFGFILE="$1"
ELFOBJ="$2"
OBJDUMP=objdump

# Every libgcc divide or modulo helper, under both its internal name and its
# EABI alias. The two patterns are separate because one cannot cover both:
#
#   __divsf3, __divsi3, __udivmodsi4   start with __ then run straight into div
#   __aeabi_fdiv, __aeabi_uidiv        have an underscore in the way, and \< can
#                                      only anchor at the start of the word
#
# The second pattern is not cosmetic. On Cortex-M0 a soft-float divide links
# __aeabi_fdiv with no __divsf3 alias at all, so for as long as only the first
# pattern existed, stm32F072, stm32f070 and stm32g0b1 passed this check while
# dividing in software once per load cell sample.
#
# __aeabi_idiv0 and __aeabi_ldiv0 are excluded on purpose: they are the
# divide-by-zero trap handlers, always linked, and they never divide.
DIV_RE='\<(__[a-z0-9]*div|__[a-z0-9]*mod)[a-z0-9_]*'
AEABI_RE='__aeabi_[a-z0-9]*(div|mod)[a-z0-9]*'

FOUND=$( { objdump -t ${ELFOBJ} | grep -Eo "${DIV_RE}" ;
           objdump -t ${ELFOBJ} | grep -Eo "${AEABI_RE}" ; } \
         | grep -vE '^__aeabi_(idiv0|ldiv0)$' | sort -u )

if [ -n "${FOUND}" ]; then

    if grep -Eq '^CONFIG_HAVE_SOFTWARE_DIVIDE_REQUIRED=y$' ${CFGFILE}; then
        echo ""
        echo "Software divide detected and that is normal for this chip"
        echo "${FOUND}" | sed 's/^/    /'
        echo ""
        exit 0
    fi

    echo ""
    echo "ERROR: A software run-time divide operation was found"
    echo "${FOUND}" | sed 's/^/    /'
    echo ""
    exit 99
fi

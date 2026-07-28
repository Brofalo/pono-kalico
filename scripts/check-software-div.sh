#!/bin/bash
# Check if a binary appears to have a software library divide operator

CFGFILE="$1"
ELFOBJ="$2"
OBJDUMP=objdump

# The historical pattern. It matches the libgcc internal names (__divsf3,
# __divsi3, __udivmodsi4) but NOT the EABI aliases, because \< anchors at the
# start of the word and '_' is a word character, so the '_' in '__aeabi_fdiv'
# breaks the [a-z0-9]* run before it can reach 'div'.
DIV_RE='\<(__[a-z0-9]*div|__[a-z0-9]*mod)'

# Same helpers under their EABI names. On Cortex-M0 a soft-float divide links
# only __aeabi_fdiv, with no __divsf3 alias, so the pattern above sees nothing
# and the check passes while the image still divides in software. Reported
# separately rather than failed on, so that turning it into an error is a
# deliberate step and not a surprise.
AEABI_RE='__aeabi_[a-z0-9]*(div|mod)[a-z0-9]*'

FOUND=$(objdump -t ${ELFOBJ} | grep -Eo "${DIV_RE}[a-z0-9_]*" | sort -u)
AEABI=$(objdump -t ${ELFOBJ} | grep -Eo "${AEABI_RE}" | sort -u \
        | grep -vE '_(idiv0|ldiv0)$')

if [ -n "${AEABI}" ] && [ -z "${FOUND}" ]; then
    echo ""
    echo "NOTE: no libgcc divide symbol matched, but these EABI divide helpers"
    echo "are linked and the pattern above cannot see them:"
    echo "${AEABI}" | sed 's/^/    /'
    echo ""
fi

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

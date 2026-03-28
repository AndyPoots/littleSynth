/* Source/glibc_compat.c
 *
 * Compatibility shims for running on GLIBC >= 2.17 (e.g., Ardour snap on
 * Ubuntu 22.04 with GLIBC 2.35).
 *
 * GCC 14 + GLIBC 2.38 headers redirect strtol/strtoul/sscanf to their C23
 * variants (__isoc23_*) which require GLIBC 2.38. GLIBC 2.38 also bumped the
 * version of fmod in libm. This file provides local definitions that delegate
 * to the older symbol versions, eliminating the GLIBC_2.38 import requirement.
 *
 * Do NOT include <stdlib.h>, <stdio.h>, or <math.h> here — they contain the
 * isoc23 redirects we are trying to avoid.
 */

#include <stdint.h>
#include <stdarg.h>

/* ------------------------------------------------------------------ */
/* strtol / strtoul / strtoumax                                       */
/* ------------------------------------------------------------------ */

extern long int          __strtol_glibc225  (const char *, char **, int);
extern unsigned long int __strtoul_glibc225 (const char *, char **, int);
extern uintmax_t         __strtoumax_glibc225(const char *, char **, int);

__asm__(".symver __strtol_glibc225,   strtol@GLIBC_2.2.5");
__asm__(".symver __strtoul_glibc225,  strtoul@GLIBC_2.2.5");
__asm__(".symver __strtoumax_glibc225,strtoumax@GLIBC_2.2.5");

long int __isoc23_strtol(const char *n, char **e, int b)
{
    return __strtol_glibc225(n, e, b);
}

unsigned long int __isoc23_strtoul(const char *n, char **e, int b)
{
    return __strtoul_glibc225(n, e, b);
}

uintmax_t __isoc23_strtoumax(const char *n, char **e, int b)
{
    return __strtoumax_glibc225(n, e, b);
}

/* ------------------------------------------------------------------ */
/* sscanf — forwarded via vsscanf to handle variadic arguments        */
/* ------------------------------------------------------------------ */

extern int __vsscanf_glibc225(const char *, const char *, va_list);
__asm__(".symver __vsscanf_glibc225, vsscanf@GLIBC_2.2.5");

int __isoc23_sscanf(const char *s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = __vsscanf_glibc225(s, fmt, ap);
    va_end(ap);
    return r;
}

/* ------------------------------------------------------------------ */
/* fmod — provide local definition to avoid libm GLIBC_2.38 import    */
/* ------------------------------------------------------------------ */

extern double __fmod_glibc225(double, double);
__asm__(".symver __fmod_glibc225, fmod@GLIBC_2.2.5");

double fmod(double x, double y)
{
    return __fmod_glibc225(x, y);
}

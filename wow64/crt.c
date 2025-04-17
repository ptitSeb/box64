#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

int __mingw_sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    int ret;

    va_start(args, format);
    ret = vsprintf(buffer, format, args);
    va_end(args);

    return ret;
}

int __isnanf(float x)
{
    union { float x; unsigned int i; } u = { x };
    return (u.i & 0x7fffffff) > 0x7f800000;
}

int __isnan(double x)
{
    union { double x; unsigned __int64 i; } u = { x };
    return (u.i & ~0ull >> 1) > 0x7ffull << 52;
}

double math_error(int type, const char *name, double arg1, double arg2, double retval)
{
    return retval;
}

int __fpclassify(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i>>52 & 0x7ff;
	if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0x7ff) return u.i<<12 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}

int __fpclassifyf(float x)
{
	union {float f; uint32_t i;} u = {x};
	int e = u.i>>23 & 0xff;
	if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0xff) return u.i<<9 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}

int fegetround (void)
{
    return 0;
}

int fesetround (int __rounding_direction)
{
    return 0;
}

div_t __cdecl div(int num, int denom)
{
    div_t ret;

    ret.quot = num / denom;
    ret.rem = num % denom;
    return ret;
}

ldiv_t __cdecl ldiv(long num, long denom)
{
    ldiv_t ret;

    ret.quot = num / denom;
    ret.rem = num % denom;
    return ret;
}

void _assert (const char *_Message, const char *_File, unsigned _Line)
{
    // NYI
}
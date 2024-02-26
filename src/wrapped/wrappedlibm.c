#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <complex.h>
#include <math.h>
#include <fenv.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "emu/x64emu_private.h"

const char* libmName =
#ifdef ANDROID
    "libm.so"
#else
    "libm.so.6"
#endif
    ;

#define LIBNAME libm

static library_t* my_lib = NULL;

typedef float   (*fFff_t)   (float, float);
typedef double  (*dFdd_t)   (double, double);
typedef float   (*fFf_t)    (float);
typedef double  (*dFd_t)    (double);

#undef GO_cFc

#define FINITE(N, T, R, P, ...)     \
EXPORT R my___##N##_finite P        \
{                                   \
    static int check = 0;           \
    T f = NULL;                     \
    if(!check) {                    \
        f = (T)dlsym(my_lib->w.lib, "__" #N "_finite");  \
        ++check;                    \
    }                               \
    if(f)                           \
        return f(__VA_ARGS__);      \
    else                            \
        return N (__VA_ARGS__);     \
}

#define F1F(N) FINITE(N, fFf_t, float, (float a), a)
#define F1D(N) FINITE(N, dFd_t, double, (double a), a)
#define F2F(N) FINITE(N, fFff_t, float, (float a, float b), a, b)
#define F2D(N) FINITE(N, dFdd_t, double, (double a, double b), a, b)

F2F(powf)
F2D(pow)
F1F(sinhf)
F1D(sinh)
F1F(sqrtf)
F1D(sqrt)
F1F(acosf)
F1D(acos)
F1F(acoshf)
F1D(acosh)
F1F(asinf)
F1D(asin)
F2F(atan2f)
F2D(atan2)
F1F(coshf)
F1D(cosh)
F1F(exp2f)
F1D(exp2)
F1F(expf)
F1D(exp)
F2F(hypotf)
F2D(hypot)
F1F(log10f)
F1D(log10)
F1F(log2f)
F1D(log2)
F1F(logf)
F1D(log)
F2F(fmodf)
F2D(fmod)

#undef F2D
#undef F2F
#undef F1D
#undef F1F
#undef FINITE

// x86-64
// FE_TONEAREST     0x0
// FE_DOWNWARD      0x400
// FE_UPWARD        0x800
// FE_TOWARDZERO    0xc00

#if defined(__aarch64__)
// AArch64
// #define FE_TONEAREST  0x000000
// #define FE_DOWNWARD   0x800000
// #define FE_UPWARD     0x400000
// #define FE_TOWARDZERO 0xc00000
#define TO_NATIVE(round) ((round == 0x400 ? 0x800 : (round == 0x800 ? 0x400 : round)) << 12)
#elif defined(__riscv)
// RISC-V
// #define FE_TONEAREST     0x0
// #define FE_DOWNWARD      0x2
// #define FE_UPWARD        0x3
// #define FE_TOWARDZERO    0x1
#define TO_NATIVE(round) ((round == 0xc00 ? 0x400 : (round == 0x0 ? round : round + 0x400)) >> 10)
#elif defined(__loongarch64)
// LOONGARCH
// FE_TONEAREST     0x000
// FE_DOWNWARD      0x300
// FE_UPWARD        0x200
// FE_TOWARDZERO    0x100
#define TO_NATIVE(round) ((round == 0x400 ? 0xc00 : (round == 0xc00 ? 0x400 : round)) >> 2)
#elif defined(__powerpc64__)
// PPC
// FE_TONEAREST     0x0
// FE_DOWNWARD      0x3
// FE_UPWARD        0x2
// FE_TOWARDZERO    0x1
#define TO_NATIVE(round) ((round == 0x400 ? 0xc00 : (round == 0xc00 ? 0x400 : round)) >> 10)
#elif defined(__x86_64__)
#define TO_NATIVE(round) round
#else
#error Unknown architecture!
#endif

// See https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/fpu/fesetround.c
EXPORT int my_fesetround(x64emu_t* emu, int round)
{
    if (box64_sync_rounding) {
        if ((round & ~0xc00) != 0)
            // round is not valid.
            return 1;

        emu->cw.x16 &= ~0xc00;
        emu->cw.x16 |= round;

        emu->mxcsr.x32 &= ~0x6000;
        emu->mxcsr.x32 |= round << 3;

        return 0;
    } else {
        return fesetround(round);
    }
}

// See https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/fpu/fegetround.c
EXPORT int my_fegetround(x64emu_t* emu)
{
    if (box64_sync_rounding) {
        return emu->cw.x16 & 0xc00;
    } else {
        return fegetround();
    }
}

#define FROUND(N, T, R)                      \
    EXPORT R my_##N(x64emu_t* emu, T val)    \
    {                                        \
        if (box64_sync_rounding) {           \
            int round = emu->cw.x16 & 0xc00; \
            fesetround(TO_NATIVE(round));    \
        }                                    \
        return N(val);                       \
    }

FROUND(rint, double, double)
FROUND(rintf, float, float)
FROUND(lrint, double, int)
FROUND(lrintf, float, int)
FROUND(llrint, double, long)
FROUND(llrintf, float, long)
FROUND(nearbyint, double, double)
FROUND(nearbyintf, float, float)
#ifdef HAVE_LD80BITS
FROUND(llrintl, long double, long double)
#else
EXPORT double my_llrintl(x64emu_t* emu, double val)
{
    if (box64_sync_rounding) {
        int round = emu->cw.x16 & 0xc00;
        fesetround(TO_NATIVE(round));
    }
    return llrint(val);
}
#endif

#ifdef ANDROID
double my_pow10(double a) { return pow(10.0, a);}
float my_pow10f(float a) { return powf(10.0f, a);}
long double my_pow10l(long double a) { return powl(10.0, a);}
#else
double my_pow10(double a) { return exp10(a);}
float my_pow10f(float a) { return exp10f(a);}
long double my_pow10l(long double a) { return exp10l(a);}
#endif

#ifdef STATICBUILD
//extern void* _LIB_VERSION;
#endif

#undef FROUND
#undef TO_NATIVE

#define CUSTOM_INIT \
    my_lib = lib;

#define CUSTOM_FINI     \
    my_lib = NULL;

#include "wrappedlib_init.h"

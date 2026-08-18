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

const char* libmName = "libm.so.6";
#define ALTNAME "libm.so"

#define LIBNAME libm

static library_t* my_lib = NULL;

typedef float   (*fFff_t)   (float, float);
typedef double  (*dFdd_t)   (double, double);
typedef float   (*fFf_t)    (float);
typedef double  (*dFd_t)    (double);
#ifdef HAVE_LD80BITS
typedef long double (*DFDD_t) (long double, long double);
#endif

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

#ifdef HAVE_LD80BITS
FINITE(powl, DFDD_t, long double, (long double a, long double b), a, b)
#else
EXPORT double my___powl_finite(double a, double b) __attribute__((alias("my___pow_finite")));
#endif

#undef F2D
#undef F2F
#undef F1D
#undef F1F
#undef FINITE

#define X64_FE_TONEAREST     0x0
#define X64_FE_DOWNWARD      0x400
#define X64_FE_UPWARD        0x800
#define X64_FE_TOWARDZERO    0xc00
static int round_to_native(int round) {
    switch(round) {
        case X64_FE_TONEAREST: return FE_TONEAREST;
        case X64_FE_DOWNWARD: return FE_DOWNWARD;
        case X64_FE_UPWARD: return FE_UPWARD;
        case X64_FE_TOWARDZERO: return FE_TOWARDZERO;
        default: 
            //should warn
            return round;
    }
}
#define TO_NATIVE(R) round_to_native(R)

static int round_from_native(int round) {
    switch(round) {
        case FE_TONEAREST:  return X64_FE_TONEAREST;
        case FE_DOWNWARD:   return X64_FE_DOWNWARD;
        case FE_UPWARD:     return X64_FE_UPWARD;
        case FE_TOWARDZERO: return X64_FE_TOWARDZERO;
        default:
            //should warn
            return round;
    }
}
#define FROM_NATIVE(R) round_from_native(R)

static int x86_to_native_excepts(int e) {
    int n = 0;
    if (e & 0x01) n |= FE_INVALID;
    if (e & 0x04) n |= FE_DIVBYZERO;
    if (e & 0x08) n |= FE_OVERFLOW;
    if (e & 0x10) n |= FE_UNDERFLOW;
    if (e & 0x20) n |= FE_INEXACT;
    return n;
}

static int native_to_x86_excepts(int e) {
    int x = 0;
    if (e & FE_INVALID)    x |= 0x01;
    if (e & FE_DIVBYZERO)  x |= 0x04;
    if (e & FE_OVERFLOW)   x |= 0x08;
    if (e & FE_UNDERFLOW)  x |= 0x10;
    if (e & FE_INEXACT)    x |= 0x20;
    return x;
}

EXPORT int my_feraiseexcept(x64emu_t* emu, int e){
    return feraiseexcept(x86_to_native_excepts(e));
}

EXPORT int my_feclearexcept(x64emu_t* emu, int e) {
    return feclearexcept(x86_to_native_excepts(e));
}

EXPORT int my_fetestexcept(x64emu_t* emu, int e) {
    int r = fetestexcept(x86_to_native_excepts(e));
    return native_to_x86_excepts(r);
}

// x86_64 FE_DFL_ENV = ((const fenv_t *) -1)
// x86_64 FE_NOMASK_ENV = ((const fenv_t *) -2)
// On some architectures (e.g. ppc64le), these are real pointers, not sentinels,
// so passing x86 sentinel values directly would cause a SIGSEGV.
// Translate x86 sentinel values to the native equivalents on all architectures.
static const fenv_t* x86_to_native_fenv(const fenv_t* envp)
{
    if (envp == (const fenv_t*)(intptr_t)-1)
        return FE_DFL_ENV;
#ifdef FE_NOMASK_ENV
    if (envp == (const fenv_t*)(intptr_t)-2)
        return FE_NOMASK_ENV;
#endif
    return envp;
}

EXPORT int my_fesetenv(x64emu_t* emu, const fenv_t* envp)
{
    return fesetenv(x86_to_native_fenv(envp));
}

EXPORT int my_feupdateenv(x64emu_t* emu, const fenv_t* envp)
{
    return feupdateenv(x86_to_native_fenv(envp));
}

// See https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/fpu/fesetround.c
EXPORT int my_fesetround(x64emu_t* emu, int round)
{
    if (BOX64ENV(sync_rounding)) {
        if ((round & ~0xc00) != 0)
            // round is not valid.
            return 1;

        emu->cw.x16 &= ~0xc00;
        emu->cw.x16 |= round;

        emu->mxcsr.x32 &= ~0x6000;
        emu->mxcsr.x32 |= round << 3;

        return 0;
    } else {
        return fesetround(TO_NATIVE(round));
    }
}

// See https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/fpu/fegetround.c
EXPORT int my_fegetround(x64emu_t* emu)
{
    if (BOX64ENV(sync_rounding)) {
        return emu->cw.x16 & 0xc00;
    } else {
        return FROM_NATIVE(fegetround());
    }
}

#define FROUND(N, T, R)                      \
    EXPORT R my_##N(x64emu_t* emu, T val)    \
    {                                        \
        if (BOX64ENV(sync_rounding)) {           \
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
FROUND(llrintl, long double, long long)
#else
EXPORT int64_t my_llrintl(x64emu_t* emu, double val)
{
    if (BOX64ENV(sync_rounding)) {
        int round = emu->cw.x16 & 0xc00;
        fesetround(TO_NATIVE(round));
    }
    return (int64_t)llrint(val);
}
EXPORT double my_nexttoward(x64emu_t* emu, double val, double to)
{
    return nexttoward(val, to);
}
EXPORT float my_nexttowardf(x64emu_t* emu, float val, double to)
{
    return nexttowardf(val, to);
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

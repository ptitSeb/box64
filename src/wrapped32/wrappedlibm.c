#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <complex.h>
#include <math.h>

#include "wrappedlibs.h"

#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "box32.h"

static const char* libmName =
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
#if 0
typedef union my_float_complex_s {
    float complex   f;
    uint64_t        u64;
} my_float_complex_t;

// complex <- FUNC(complex) wrapper
#define GO_cFc(N)                                   \
EXPORT void* my_##N(void* p, void* c)               \
{                                                   \
    *(double complex*)p = N(*(double complex*)c);   \
    return p;                                       \
}                                                   \
EXPORT uint64_t my_##N##f(void* c)                  \
{                                                   \
    my_float_complex_t ret;                         \
    ret.f = N##f(*(float complex*)c);               \
    return ret.u64;                                 \
}
// complex <- FUNC(complex, complex) wrapper
#define GO_cFcc(N)                                  \
EXPORT void* my_##N(void* p, void* c, void* d)      \
{                                                   \
    *(double complex*)p = N(*(double complex*)c, *(double complex*)d);   \
    return p;                                       \
}                                                   \
EXPORT uint64_t my_##N##f(void* c, void* d)         \
{                                                   \
    my_float_complex_t ret;                         \
    ret.f = N##f(*(float complex*)c, *(float complex*)c);               \
    return ret.u64;                                 \
}

GO_cFc(clog)
GO_cFc(csqrt)
GO_cFc(cproj)
GO_cFc(cexp)
GO_cFc(ccos)
GO_cFc(csin)
GO_cFc(ccosh)
GO_cFc(csinh)
GO_cFc(ctan)
GO_cFc(ctanh)
GO_cFc(cacos)
GO_cFc(casin)
GO_cFc(cacosh)
GO_cFc(casinh)
GO_cFc(catan)
GO_cFc(catanh)
GO_cFcc(cpow)

#undef GO_cFc
#undef GO_cFcc
#endif

#define FINITE(N, T, R, P, ...)     \
EXPORT R my32___##N##_finite P      \
{                                   \
    static int check = 0;           \
    static T f = NULL;              \
    if(!check) {                    \
        f = (T)dlsym(my_lib->w.lib, "__" #N "_finite");  \
        ++check;                    \
    }                               \
    if(f)                           \
        return f(__VA_ARGS__);      \
    else                            \
        return N(__VA_ARGS__);      \
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

#undef F2D
#undef F2F
#undef F1D
#undef F1F
#undef FINITE

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_INIT     \
    my_lib = lib;

#define CUSTOM_FINI     \
    my_lib = NULL;

#include "wrappedlib_init32.h"

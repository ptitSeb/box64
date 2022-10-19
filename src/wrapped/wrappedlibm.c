#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <complex.h>
#include <math.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"

const char* libmName = "libm.so.6";
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


#define CUSTOM_INIT     \
    my_lib = lib;

#define CUSTOM_FINI     \
    my_lib = NULL;

#include "wrappedlib_init.h"

